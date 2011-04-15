<?php

/*
 * contains methods to search solr and display results.  depends on Apache_Solr_Php client.
 */

module_load_include('inc', 'islandora_solr_search', 'IslandoraSolrResults');

/**
 * Extention of IslandoraSolrResults for templating purposes.
 * This overrides the displayResults function to provide an alternate display type.
 */

class IslandoraSolrResultsCSV extends IslandoraSolrResults {

  function IslandoraSolrResultsCSV() {
    module_load_include('php', 'islandora_solr_search', 'SolrPhpClient/Apache/Solr/Service');
  }

  /**
   * Outputs results basically in the normal way, but with thumbnails pulled
   * from the Fedora repository.
   *
   * @param $results
   *   A solr resultset object.
   *
   * @return
   *   html output for the resultset. Note: we currently create this
   *   output manually, should refactor to use drupal forms api.
   */
  function printCSV($solrQueryProcessor, $title = "Search Results", $output = '') {

    // First off, update limits and re-query
    $solrQueryProcessor->solrLimit = $solrQueryProcessor->solrResult->response->numFound;
    $solrQueryProcessor->solrStart = 0;

    $solrQueryProcessor->executeQuery();

    $results = $solrQueryProcessor->solrResult;

    global $base_url;
    if(empty($results)) {
      return "";
    }
    $row_count = 0;
    $field_counts = array();
    $values = array();

    $seperator = ",";
    $wrap = '"';
    $replace_value = '""';
    

    foreach($results->response->docs as $doc) {
      foreach($doc as $field => $value) {

        // Sometimes we get a single value, sometimes an array.
        // For simplicity, I push the singles into an array and later on process each
        // field as an array.
        if(!is_array($value)) {
          $value = array($value);
        }
        
        $field_count = count($value);

        if( !isset( $field_counts[$field] )) {
          $field_counts[$field] = $field_count;
        }
        else {
          $field_counts[$field] = max($field_counts[$field], $field_count);
        }

        $values[$row_count][$field] = $value;
      }
      $row_count++;
    }


    $rows = array();
    $row = 0;

    // Generate Header
    foreach( $field_counts as $field=>$count ) {
      if( $count == 1 ) {
        $header[] = $wrap . str_replace('"', $replace_value, $field) . $wrap;
      }
      else {
        $i = 1;
        while( $i <= $count ) {
          $header[] = $wrap . str_replace('"', $replace_value, $field."_".$i) . $wrap;
          $i++;
        }
      }
    }

    foreach( $values as $row_count => $val_array ) {
      foreach( $field_counts as $field=>$count ) {
        $i = 0;
        while( $i < $count ) {
          if( isset( $val_array[$field][$i] )) {
            $rows[$row_count][] = $wrap . str_replace('"', $replace_value, $val_array[$field][$i]) . $wrap;
          }
          else {
            $rows[$row_count][] = "";
          }
          $i++;
        }
      }
    }

    $output = array(
      'header' => $header,
      'rows' => $rows,
    );

    drupal_set_header('Content-Type: text/csv');
    drupal_set_header('Content-Disposition: attachment; filename="searchresults.csv"');

    print implode($seperator, $header) . "\r\n";
    foreach( $rows as $count => $row ) {
      print implode($seperator, $row) . "\r\n";
    }

    exit;

  }

}


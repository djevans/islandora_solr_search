<?php

/*
 * contains methods to search solr and display results.  depends on Apache_Solr_Php client.
 */


/**
 * Extention of IslandoraSolrResults for templating purposes.
 * This overrides the displayResults function to provide an alternate display type.
 */

class IslandoraSolrResultsTable extends IslandoraSolrResults {

  static $facetSeparator = '~';//used to separate facets in url
  static $slashReplacement = '~slsh~';// a pattern used to replace / in url's the slash breaks drupal clean url's
  public static $SEARCH_CLASS_ADVANCED_SEARCH_NUMBER_FIELDS = 5;
  function IslandoraSolrResults() {
    module_load_include('php', 'islandora_solr_search', 'Solr/Service');
  }

  /**
   * Outputs results in unified tabular format.
   *
   * This function collects all field names from all results in the 
   * resultset and constructs a single table with said field names in 
   * a unified header row, rather than outputting each result as a 
   * miniature horizontal table.
   *
   * @param $results
   *   A solr resultset object.
   *
   * @return
   *   html output for the resultset. Note: we currently create this 
   *   output manually, should refactor to use drupal forms api.
   */
  function displayResults($results){
    global $base_url;
    $recordStart = (int)$results->response->start;
    if(empty($results)) {
      return "no results";
    }
    $count = 0;
    $fields = array();
    $values = array();

    $object_link_placeholder = "~object_href~";

    foreach($results->response->docs as $doc) {
      unset($pid);
      foreach($doc as $field => $value) {

        // catches the object PID.
        if($field=='PID') {
          $pid = $value;
          // we need this in the resultset, but would prefer to hide it from users.
        }

        // Sometimes we get a single value, sometimes an array.  
        // For simplicity, I push the singles into an array and later on process each
        // field as an array.
        if(!is_array($value)) {
          $value = array($value);
        }

        // We're going to put the object link onto the dublin-core title.
        // Since we haven't necessarily caught the pid yet, we use the pid placeholder and 
        // str_replace later on.
        if($field=='dc.title' || $field=='dc:title') {
          foreach ($value as $key=>$val) {
            if(isset($doc->PID)) {
              $value[$key] = '<a href="'.$base_url.'/fedora/repository/'.$doc->PID.'/-/'.$val.'">'.$val.'</a>';
            }
          }
        }
        $value = implode(", ",$value);

        //this would change 'animal_name' to the more readable 'Animal Name'
        //$field = ucwords( str_replace( '_', ' ', $field ) );

        // Add this field to the master list of fields if it's not there already:
        if (!in_array( $field, $fields )) {
          $fields[] = $field;
        }

        $values[$count][$field] = $value;
      }
      $count++;
    }

    $output .= '<table class="islandora_solr_search_results"><thead><tr><th>#</th>';
    foreach($fields as $field) {
      $output .= '<th>'.$field.'</th>';
    }
    $output .= '</tr></thead>';
    $output .= '<tbody>';
    foreach ($values as $num => $val_array) {
      unset($rowclass);
      $rowclass[] = ( $num%2 ? 'even' : 'odd' );
      if( $num == 0 ) $rowclass[] = 'first';
      $output .= '<tr class="'. implode(" ",$rowclass) .'">';
      $result_num = $recordStart + $num + 1;
      $output .= '<th>'. (isset($val_array['PID']) ? l($result_num,$base_url.'/fedora/repository/'.$val_array['PID']) : $result_num) .'</th>';
      foreach ($fields as $field) {
        if (!empty ($val_array[$field])) {
          $output .= '<td>'.$val_array[$field].'</td>';
        } else {
          $output .= '<td></td>';
        }
      }
      $output .= '</tr>';
    }
    $output .= '</tbody>';
    $output .= '</table>';

    if( variable_get('islandora_solr_search_debug_mode', 0) ) { // debug dump
      $results_r .= "<pre>Results: ".print_r($results,true)."</pre>";
      $fieldset_r = array(
        '#title' => t("Raw Results"),                                                                                                                                             
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
        '#value' => $results_r,
        );
      $output .= theme('fieldset', $fieldset_r);
    }
    return $output;
  }

}


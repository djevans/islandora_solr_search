<?php

/*
 * Contains methods to build and execute a solr query.
 * Depends on Apache_Solr_Php client.
 */


// TODO: Stop using global vars, start using this object's vars.
// The module itself should initially create an instance of this
// object and reference the query vars inside that object when calling
// other output display functions.

class IslandoraSolrQueryProcessor {

  static $facetSeparator = '~';//used to separate facets in url
  static $slashReplacement = '~slsh~';// a pattern used to replace / in url's the slash breaks drupal clean url's
  public static $SEARCH_CLASS_ADVANCED_SEARCH_NUMBER_FIELDS = 5;
  public $solrQuery;
  public $solrParams;
  public $solrStart;
  public $solrLimit;
  public $solrDefType;
  public $solrFacets;
  public $solrFilters;
  public $solrResult;

  function IslandoraSolrQueryProcessor() {
    module_load_include('php', 'islandora_solr_search', 'SolrPhpClient/Apache/Solr/Service');
    module_load_include('inc', 'islandora_solr_search', 'includes/common');
  }


  //default implementation for solr search.  You can configure others in the block config settings
  function buildQuery($query, $fq=null, $dismax=null) {
    // Get pager variable
    $startPage = isset($_GET['page']) ? $_GET['page'] : 0;
    $this->solrLimit = variable_get('islandora_solr_search_num_of_results', 20);
    global $base_url, $facetArray; //used by facet block as well

    // Get display profile
    $islandora_solr_primary_display = variable_get( 'islandora_solr_primary_display', 'default' );
    $profiles = module_invoke_all("islandora_solr_primary_display");
    if( !isset( $profiles[$islandora_solr_primary_display] )) {
      drupal_set_message("There is an error in the solr search configuration: the display profile is not found.", 'error');
      return "";
    }
    $profile = $profiles[$islandora_solr_primary_display];

    // Include the file for the display profile
    require_once(drupal_get_path('module', $profile['module']) .'/'. $profile['file']);

    // Set display class and function vars
    $solrClass = $profile['class'];
    $solrFunction = $profile['function'];

    $this->solrQuery = replaceSlashes($query); //replace the slash so we can use this in a url without breaking drupal clean urls
    if (empty($this->solrQuery)) {
//      $this->solrQuery = '%20'; //so we can allow empty queries to dismax
    }

    $url = variable_get('islandora_solr_search_block_url', 'http://localhost:8080/solr');
    $pathParts = parse_url($url);
    $facetArray = array();
    $facetFields = '';
    $rawFacetVals = variable_get("islandora_solr_search_block_facets", 'dc.subject,dc.type');
    $lines = preg_split('/[,|\n|\r]/', $rawFacetVals);
    foreach ($lines as $line) {
      $lineArray = explode('~', $line);
      $key = trim($lineArray[0]);
      $value = trim($lineArray[1]);
      if (!$value) {
        $value = $key;
      }
      $facetArray[$key] = $value;
      if ($facetFields) {
        $facetFields .= ",";
      }
      $facetFields .= $key;
    }
    $solr = new Apache_Solr_Service($pathParts['host'], $pathParts['port'], $pathParts['path'] . '/');

    //fix the query as some characters will break the search : and / slash are examples
//    $this->solrQuery = restoreSlashes($query);


    $facetlimit = variable_get('islandora_solr_search_block_facet_limit', '12');
    $facetMinCount = variable_get('islandora_solr_search_block_facet_min_count', '2');
    $requestHandler = variable_get("islandora_solr_search_block_request_handler", "standard");
    $this->solrParams = array(
      'facet' => 'true',
      'facet.mincount' => $facetMinCount,
      'facet.limit' => $facetlimit,
      'qt' => $requestHandler,
      'facet.field' => explode(',', $facetFields), //comma separated list configured in the block config
    );


    $debugQuery = (variable_get('islandora_solr_search_debug_mode', 0) ? "true" : null ); //any val incl. 'false' is treated as true by Solr

    global $islandora_fq;
    if ($fq != NULL && $fq != '-') {
      $fq = restoreSlashes($fq); //put the slash back
      $fqs = csv_explode(IslandoraSolrQueryProcessor::$facetSeparator,$fq,'"',true);//to filter by more then one facet we will separate them by~ for nowseparate them by~ for now
      $this->solrParams['fq'] = $fqs;
      $islandora_fq = replaceSlashes($fq); //remove the slash here as we will be using this in url's
    }
    if (empty($islandora_fq)) {
      $islandora_fq = '-';
    }
    if ($dismax != NULL) {
      $this->solrDefType = $dismax;
      $this->solrParams['defType'] = $dismax;
    }
    $this->solrStart = max(0, $startPage) * $this->solrLimit;

    $queryurl = "islandora/solr/search/$this->solrQuery";
    $breadcrumb_fq = $islandora_fq;
    if (strcmp($islandora_fq,"-")) {
      foreach( csv_explode(IslandoraSolrQueryProcessor::$facetSeparator,$islandora_fq,'"',true) as $facet ) {
        $field = substr($facet,0,strpos($facet,":"));
        $value = trim(substr($facet,strpos($facet,":")+1),'"');
        $options = array('html'=>true);
        $options['attributes']['title'] = $facet;
        $crumblink = $queryurl . "/" . $breadcrumb_fq;
        $cut_fq = $this->delete_filter($islandora_fq,$facet);
        $cutlink = $queryurl . "/" . $cut_fq;
        if( !strncmp( "-", $facet, 1 )) {
          $options['attributes']['class'] = "strikethrough";
        }
        $breadcrumb[] = l($value,$crumblink,$options) .
                        " <span class='islandora_solr_search_breadcrumb_super'>(".l("x",$cutlink,array('attributes'=>array('title'=>"Remove ".$facet))).")</span>";
        $breadcrumb_fq = $this->delete_filter($breadcrumb_fq,$facet);
      }
    }
    if( !empty( $this->solrQuery ) && strcmp( $this->solrQuery, ' ' )) {
      $queryval = trim(substr($this->solrQuery,strpos($this->solrQuery,":")+1),")(");
      $breadcrumb[] = l($queryval,$queryurl."/-",array('attributes'=>array('title'=>$this->solrQuery)));
    }
    $breadcrumb[] = l(t('Home'),NULL);
    if( !empty( $breadcrumb )) $breadcrumb = array_reverse($breadcrumb);
    drupal_set_breadcrumb($breadcrumb);

    // At this point let's invoke a hook for third-party modules to mess about
    // with the query parameters if they need to.  Third party modules may alter
    // this object's query parameters if they wish.
    module_invoke_all("islandora_solr_search_query_processor", &$this);


    // //////////////////////////////////////////////////////////////////////////// //
    // Right here the function should be split.  One function to execute the query, //
    // another to set up pager variables and generate display.                      //
    // //////////////////////////////////////////////////////////////////////////// //

    // This is where the query gets executed and output starts being created.
    try {
      $results = $solr->search($this->solrQuery, $this->solrStart, $this->solrLimit, $this->solrParams);
    } catch (Exception $e) {
      drupal_set_message(t('error searching ') . $e->getMessage());
    }
    $this->solrResult = $results;
    if (empty($results)) {
      drupal_set_message(t('Error searching solr index. Is the solr search block configured properly?'), 'error');
      return ' ';
    }
    $total = (int) $results->response->numFound;
    $end = min(($this->solrLimit + $this->solrStart), $total);

    // Initialize drupal-style pager
    islandora_solr_search_pager_init($total, $this->solrLimit);

    // Render Drupal pager (top of search results)
    // $output .= theme('pager');
    $output .= '<h2>Search Results</h2>';
    $output .= '<div id="numresults">(' . ($this->solrStart + 1) . ' - ' . $end . ' of ' . $total . ')</div>';

    if (class_exists($solrClass)) {
      $implementation = new $solrClass();
    } else {
      drupal_set_message(t('Error loading solr results class. Class '.$solrClass.' could not be loaded. Check the Islandora Solr search blocks configuration.'), 'error');
      return ' ';
    }

    if (method_exists($implementation, $solrFunction)) {
      $output.= $implementation->$solrFunction($results);
      if (variable_get('islandora_solr_search_debug_mode', 0)) { // debug dump
        drupal_set_message('Params: <br/><pre>' . print_r($this->solrParams, true) . '</pre>', 'status');
      }
    } else {
      drupal_set_message(t('Error loading solr results class. Could not find the specified method.  Check the Islandora Solr search blocks configuration.'), 'error');
    }

    // Render Drupal pager (bottom of search results)
    $output .= theme('pager',null,$this->solrLimit,0,null,5);

    global $islandora_facets; //make facets available to the islandora facet block
    $islandora_facets = $results->facet_counts->facet_fields;
    $this->solrFacets = $results->facet_counts->facet_fields;
    $this->solrFilters = $islandora_fq;
    return $output;
  }


  /**
   * Function delete_filter
   *
   * Deletes one filter from a multi-filter string
   *
   * @param <string> $facets
   * @param <string> $facet
   */
  function delete_filter($filters,$filter) {
    $filters = str_replace($filter,"",$filters);
    $filters = str_replace("~~","~",$filters);
    $filters = trim($filters,"~");
    return $filters;
  }

}


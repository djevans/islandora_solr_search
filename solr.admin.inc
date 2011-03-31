<?php

/**
 * Function to return admin setting form
 * @return array 
 */
function islandora_solr_admin_settings(&$form_state) {

  //checks for existence of PHP Solr client.

  module_load_include('php', 'islandora_solr_search', 'Solr/Service');
  if (!class_exists(Apache_Solr_Service)) {
    drupal_set_message("This module requires the " . l('Apache Solr php client', 'http://code.google.com/p/solr-php-client') . '.
      Please install the client directory in the root directory of this module before continuing.');
    return;
  }
  $handlers['standard'] = 'standard';


  $solr_url = !empty($form_state['values']['islandora_solr_search_block_url']) ? $form_state['values']['islandora_solr_search_block_url'] : variable_get('islandora_solr_search_block_url', 'localhost:8080/solr');
  $solr_avail = solr_available($solr_url);
  if ($solr_avail) {
    $handlers = get_handlers($solr_url);
  }
  // optional config modules will use hook_alter_form to add to this array

  $config_options = array();
  //               module_name          file_name                 class_name           display_method
  $config_options['islandora_solr_search~IslandoraSolrResults.inc~IslandoraSolrResults~displayResults'] = 'Default';
  $confirmation_message = ($solr_avail ? '<img src="' . url('misc/watchdog-ok.png') . '"/>'
          . t('Successfully connected to Solr server at !islandora_solr_search_block_url', array('!islandora_solr_search_block_url' => $solr_url)) : '<img src="'
          . url('misc/watchdog-error.png') . '"/> '
          . t('Unable to connect to Solr server at !islandora_solr_search_block_url</p>', array('!islandora_solr_search_block_url' => $solr_url)));

  $form = array();

  $form['solr_ahah_wrapper'] = array(
    '#prefix' => '<div id="solr-url">',
    '#suffix' => '</div>',
    '#type' => 'fieldset',
  );

  $form['solr_ahah_wrapper']['islandora_solr_search_block_url'] = array(
    '#type' => 'textfield',
    '#title' => t('Solr url'),
    '#size' => 25,
    '#weight' => -1,
    '#description' => t('The url of the Solr installation.  Defaults to localhost:8080/solr.'),
    '#default_value' => $solr_url,
    '#ahah' => array(
      'path' => 'islandora/solr/process',
      'wrapper' => 'solr-url',
      'effect' => 'fade',
      'event' => 'blur',
    ),
  );

  $form['solr_ahah_wrapper']['refresh_page'] = array(
    '#type' => 'submit',
    '#value' => t('Test connection'),
    '#attributes' => array('class' => 'refresh-button'),
    '#submit' => array('solr_admin_refresh'),
  );

  $form['solr_ahah_wrapper']['infobox'] = array(
    '#type' => 'item',
    '#value' => $confirmation_message,
  );

  $form['solr_ahah_wrapper']['islandora_solr_search_block_request_handler'] = array(
    '#type' => 'select',
    '#title' => t('Request Handler'),
    '#options' => $handlers,
    '#description' => t('Request handlers defined by !config', array('!config' => l('solrconfig.xml', 'http://wiki.apache.org/solr/SolrConfigXml' ))),
    '#default_value' => variable_get('islandora_solr_search_block_request_handler', t('standard')),
  );

  $form['islandora_solr_config_options'] = array(
    '#type' => 'select',
    '#title' => t('Configuration package'),
    '#multiple' => false,
    '#options' => $config_options,
    '#description' => t('Name of configuration package for displaying output.'),
    '#default_value' => variable_get('islandora_solr_config_options', 'islandora_solr_search~IslandoraSolrResults.inc~IslandoraSolrResults~displayResults'),
  );
  $form['islandora_solr_search_block_repeat'] = array(
    '#type' => 'textfield',
    '#title' => t('Search Field Repetition'),
    '#size' => 5,
    '#description' => t('The number of times you would like the search blocks to be repeated'),
    '#default_value' => variable_get('islandora_solr_search_block_repeat', t('3')),
  );

  $form['islandora_solr_search_block_facets'] = array(
    '#type' => 'textarea',
    '#title' => t('Facet Fields'),
    // '#size' => 75,
    '#description' => t('A comma-separated list of fields for faceting. An optional friendly term my be added to each solr term for a more readable display. <br />
      The solr term and the friendly term must be separated with a tilde. ie <strong>dc.title ~ Title, mods.sor ~ Author</strong> '),
    '#default_value' => variable_get('islandora_solr_search_block_facets', t('dc.subject,dc.type')),
  );

  $form['islandora_solr_search_block_facet_min_count'] = array(
    '#type' => 'textfield',
    '#title' => t('Minimum facet fields responses'),
    '#size' => 5,
    '#description' => t('This param indicates the minimum counts for facet fields should be included in the response. '),
    '#default_value' => variable_get('islandora_solr_search_block_facet_min_count', '2'),
  );
  $form['islandora_solr_search_block_facet_limit'] = array(
    '#type' => 'textfield',
    '#title' => t('Maximum facet field constraints'),
    '#size' => 5,
    '#description' => t('This param indicates the maximum number of constraint counts that should be returned for the facet fields. A negative value means unlimited. '),
    '#default_value' => variable_get('islandora_solr_search_block_facet_limit', '10'),
  );
  $form['islandora_solr_search_num_of_results'] = array(
    '#type' => 'textfield',
    '#title' => t('Results per page'),
    '#size' => 5,
    '#description' => t('This param indicates the number of results to show per page. '),
    '#default_value' => variable_get('islandora_solr_search_num_of_results', '20'),
  );
  $form['islandora_solr_search_debug_mode'] = array(
    '#type' => 'checkbox',
    '#title' => t('Debug Mode?'),
    '#return_value' => 1,
    '#default_value' => variable_get('islandora_solr_search_debug_mode', 0),
    '#description' => t('Dumps solr query info to the screen for testing purposes.'),
  );

  $form['buttons']['submit'] = array('#type' => 'submit', '#value' => t('Save Solr Configuration'));
  $form['buttons']['reset'] = array('#type' => 'submit', '#value' => t('Reset to defaults'));

  if (!empty($_POST) && form_get_errors()) {
    drupal_set_message(t('The settings have not been saved because of the errors.'), 'error');
  }
  $form['#submit'][] = 'solr_settings_form_submit';
  $form['#theme'] = 'system_settings_form';


  return ($form);
}

/**
 *
 * @param String $solr_url
 * @return boolean
 *
 * Checks availability of Solr installation
 *
 */
function solr_available($solr_url) {
  // path from url is parsed to allow graceful inclusion or exclusion of 'http://'
  $pathParts = parse_url($solr_url);
  $path = 'http://' . $pathParts['host'] . ':' . $pathParts['port'] . $pathParts['path'] . '/admin/file';
  $test = @fopen($path, "r");
  if ($test) {
    return true;
  }
  return false;
}

/**
 *
 * @param String $solr_url
 * @return array
 */
function get_handlers($solr_url) {
  $url = $solr_url . '/admin/file/?file=solrconfig.xml';
  $ch = curl_init();
  curl_setopt($ch, CURLOPT_RETURNTRANSFER, TRUE);
  curl_setopt($ch, CURLOPT_URL, $url);
  $xml = simplexml_load_string(curl_exec($ch));
  foreach ($xml->requestHandler as $handler) {
    if ($handler['class'] == 'solr.SearchHandler') {
      $handlerName = (string) $handler['name'];
      $slashed = preg_match('/^\//', $handlerName);
      if ($handlerName != 'dismax' && $handlerName != 'partitioned' & !$slashed) {
        $handlers[$handlerName] = $handlerName;
      }
    }
  }
  return $handlers;
}

/**
 * Checks url validity and refreshes requestHandler dropdown list
 */
function update_solr_url_div() {
  $form = callback_prep();
  $changed_elements = $form['solr_ahah_wrapper'];
  unset($changed_elements['#prefix'], $changed_elements['#suffix']);
  $output = theme('status_messages') . drupal_render($changed_elements);
  drupal_json(array(
    'status' => TRUE,
    'data' => $output,
  ));
}

/**
 *
 * @return array
 * Utilty function to prep $form_state for ahah
 */
function callback_prep() {
  $form_state = array('storage' => NULL, 'submitted' => FALSE);
  $form_build_id = $_POST['form_build_id'];
  $form = form_get_cache($form_build_id, $form_state);
  $args = $form['#parameters'];
  $form_id = array_shift($args);
  $form_state['post'] = $form['#post'] = $_POST;
  // Enable the submit/validate handlers to determine whether AHAH-submittted.
  $form_state['ahah_submission'] = TRUE;
  $form['#programmed'] = $form['#redirect'] = FALSE;
  drupal_process_form($form_id, $form, $form_state);
  $form = drupal_rebuild_form($form_id, $form_state, $args, $form_build_id);
  return $form;
}

/**
 *
 * @param array $form
 * @param array $form_state
 * @return null
 */
function solr_settings_form_submit($form, &$form_state) {

  if ($form_state['clicked_button']['#id'] != 'edit-submit') {
    $form_state['rebuild'] = TRUE;
    return;
  }
  $op = isset($form_state['values']['op']) ? $form_state['values']['op'] : '';

  // Exclude unnecessary elements.
  unset($form_state['values']['submit'], $form_state['values']['reset'], $form_state['values']['form_id'], $form_state['values']['op'], $form_state['values']['form_token'], $form_state['values']['form_build_id']);

  foreach ($form_state['values'] as $key => $value) {
    if ($op == t('Reset to defaults')) {
      variable_del($key);
    }
    else {
      if (is_array($value) && isset($form_state['values']['array_filter'])) {
        $value = array_keys(array_filter($value));
      }
      variable_set($key, $value);
    }
  }
  if ($op == t('Reset to defaults')) {
    drupal_set_message(t('The configuration options have been reset to their default values.'));
  }
  else {
    drupal_set_message(t('The solr configuration options have been saved.'));
  }

  cache_clear_all();
  drupal_rebuild_theme_registry();
}

function solr_admin_refresh($form, &$form_state) {
  $values = $form_state['values'];
  unset($form_state['submit_handlers']);
  form_execute_handlers('submit', $form, $form_state);
  $form_state['rebuild'] = TRUE;
}
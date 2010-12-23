<?php

/**
 * Function to return admin setting form
 * @return array 
 */
function islandora_solr_admin_settings() {

  //checks for existence of PHP Solr client.

  module_load_include('php', 'islandora_solr_search', 'Solr/Service');
  if (!class_exists(Apache_Solr_Service)) {
    drupal_set_message("This module requires the " . l('Apache Solr php client', 'http://code.google.com/p/solr-php-client') . '.
      Please install the client directory in the root directory of this module before continuing.');
    return;
  }


  // optional config modules will use hook_alter_form to add to this array

  $config_options = array();
  $config_options['islandora_solr_search~IslandoraSolrResults.inc~IslandoraSolrResults~displayResults'] = 'Default';

  $form = array();
  $form['islandora_solr_config_options'] = array(
      '#type' => 'select',
      '#title' => t('Choose display configuration package'),
      '#multiple' => false,
      '#options' => $config_options,
      '#description' => t('Name of configuration package for displaying output.'),
      '#default_value' => variable_get('islandora_solr_config_options', 'islandora_solr_search~IslandoraSolrResults.inc~displayResults'),
  );
  $form['islandora_solr_search_block_repeat'] = array(
      '#type' => 'textfield',
      '#title' => t('Number of times to repeat search fields'),
      '#size' => 5,
      '#description' => t('The number of times you would like the search blocks to be repeated'),
      '#default_value' => variable_get('islandora_solr_search_block_repeat', t('3')),
  );
  $form['islandora_solr_search_block_request_handler'] = array(
      '#type' => 'textfield',
      '#title' => t('The name of the solr request handler to use for this search'),
      '#size' => 25,
      '#description' => t('The name of the solr request handler to use for this search.  This must be configured in your solr config file.  If this is not set we will use the default handler.'),
      '#default_value' => variable_get('islandora_solr_search_block_request_handler', t('standard')),
  );
  $form['islandora_solr_search_block_facets'] = array(
      '#type' => 'textarea',
      '#title' => t('The fields to use for facets'),
      // '#size' => 75,
      '#description' => t('a space separated list of fields to use as facets'),
      '#default_value' => variable_get('islandora_solr_search_block_facets', t('dc.subject dc.type')),
  );
  $form['islandora_solr_search_block_host'] = array(
      '#type' => 'textfield',
      '#title' => t('Domain name or IP address of the solr server'),
      '#size' => 25,
      '#description' => t('The domain name or ip of the solr server.  Do not include http://.  The default is localhost.'),
      '#default_value' => variable_get('islandora_solr_search_block_host', t('localhost')),
  );
  $form['islandora_solr_search_block_port'] = array(
      '#type' => 'textfield',
      '#title' => t('Solr server port'),
      '#size' => 5,
      '#description' => t('The port on which the solr server listens.  The default is 8080'),
      '#default_value' => variable_get('islandora_solr_search_block_port', t('8080')),
  );
  $form['islandora_solr_search_block_app_path'] = array(
      '#type' => 'textfield',
      '#title' => t('Path to the solr webapp'),
      '#size' => 25,
      '#description' => t('Path to the solr application within your apache webapps folder.  The default value is solr'),
      '#default_value' => variable_get('islandora_solr_search_block_app_path', t('solr')),
  );
  $form['islandora_solr_search_block_facet_min_count'] = array(
      '#type' => 'textfield',
      '#title' => t('This param indicates the minimum counts for facet fields should be included in the response'),
      '#size' => 5,
      '#description' => t('This param indicates the minimum counts for facet fields should be included in the response. '),
      '#default_value' => variable_get('islandora_solr_search_block_facet_min_count', '2'),
  );
  $form['islandora_solr_search_block_facet_limit'] = array(
      '#type' => 'textfield',
      '#title' => t('This param indicates the maximum number of constraint counts that should be returned for the facet fields. A negative value means unlimited'),
      '#size' => 5,
      '#description' => t('This param indicates the maximum number of constraint counts that should be returned for the facet fields. A negative value means unlimited. '),
      '#default_value' => variable_get('islandora_solr_search_block_facet_limit', '10'),
  );
  $form['islandora_solr_search_num_of_results'] = array(
      '#type' => 'textfield',
      '#title' => t('This param indicates the number of results to show per page'),
      '#size' => 5,
      '#description' => t('This param indicates the number of results to show per page. '),
      '#default_value' => variable_get('islandora_solr_search_num_of_results', '20'),
  );
  $form['islandora_solr_search_debug_mode'] = array(
      '#type' => 'checkbox',
      '#title' => t('Debug Mode?'),
      '#return_value' => 1,
      '#default_value' => variable_get('islandora_solr_search_debug_mode', 0),
      '#description' => t('This param determines whether a bunch of solr query info should be dumped to screen.'),
  );


  return system_settings_form($form);
}


<?php

function solr_settings_ahah_callback(){
  $url = $_POST['islandora_solr_search_block_url'];
  $solr_avail = solr_available($url);
  $output = ($solr_avail ? '<img src="' . url('misc/watchdog-ok.png') . '"/>' . t('Successfully connected to Solr server at !islandora_solr_search_block_url', array('!islandora_solr_search_block_url' => $url)) : '<img src="' . url('misc/watchdog-error.png') . '"/> ' . t('Unable to connect to Solr server at !islandora_solr_search_block_url</p>', array('!islandora_solr_search_block_url' => $url)));

  return drupal_json(array('status' => TRUE, 'data' => $output));
}


function solr_available($url) {
  // path from url is parsed to allow graceful inclusion or exclusion of 'http://'
 
  $pathParts = parse_url($url);
  $path = 'http://' . $pathParts['host'] . ':' . $pathParts['port'] . $pathParts['path'];
  $test = @fopen($path, "r");
  if ($test) {
    return true;
  }
  return false;
}

function replace_url_div() {
  $form_state = array('storage' => NULL, 'submitted' => FALSE);
  $form_build_id = $_POST['form_build_id'];
  $form = form_get_cache($form_build_id, $form_state);

  $args = $form['#parameters'];
  $form_id = array_shift($args);
  $form_state['post'] = $form['#post'] = $_POST;
  $form['#programmed'] = $form['#redirect'] = FALSE;
  $form['solr_ahah_wrapper']['infobox']['#value'] = 'changed';
  dsm($form);

  drupal_process_form($form_id, $form, $form_state);
  $form = drupal_rebuild_form($form_id, $form_state, $args, $form_build_id);

  $wrapper = $form['solr_ahah_wrapper'];
  $output = drupal_render($wrapper);

  // Final rendering callback.
  print drupal_json(array('status' => TRUE, 'data' => $output));
  exit();
}
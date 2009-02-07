<?php
// $Id$

/**
 * @file
 * Administrative page callbacks for the linkchecker module.
 */

function linkchecker_admin_settings_form(&$form_state) {

  $form['settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('General settings'),
    '#collapsible' => FALSE,
  );
  $form['settings']['linkchecker_scan_nodetypes'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Scan node types for links'),
    '#default_value' => variable_get('linkchecker_scan_nodetypes', array()),
    '#options' => array_map('check_plain', node_get_types('names')),
    '#description' => t('Enable link checking for the selected node type(s).'),
  );

  $comment_dependencies = '<div class="admin-dependencies">';
  $comment_dependencies .= t('Depends on: !dependencies', array('!dependencies' => (module_exists('comment') ? t('@module (<span class="admin-enabled">enabled</span>)', array('@module' => 'Comment')) : t('@module (<span class="admin-disabled">disabled</span>)', array('@module' => 'Comment')))));
  $comment_dependencies .= '</div>';

  $form['settings']['linkchecker_scan_comments'] = array(
    '#default_value' => variable_get('linkchecker_scan_comments', 0),
    '#type' => 'checkbox',
    '#title' => t('Scan comments for links'),
    '#description' => t('Enable this checkbox if links in comments of the above selected node type(s) should be checked.') . $comment_dependencies,
    '#disabled' => module_exists('comment') ? FALSE : TRUE,
  );
  $form['settings']['linkchecker_scan_blocks'] = array(
    '#default_value' => variable_get('linkchecker_scan_blocks', 0),
    '#type' => 'checkbox',
    '#title' => t('Scan blocks for links'),
    '#description' => t('Enable this checkbox if links in blocks should be checked.')
  );
  $form['settings']['linkchecker_fqdn_only'] = array(
    '#default_value' => variable_get('linkchecker_fqdn_only', 1),
    '#type' => 'checkbox',
    '#title' => t('Check full qualified domain names only'),
    '#description' => t('Enable this checkbox if only full qualified URLs should be checked. If unchecked, all internal and external URLs will be checked.')
  );

  $pathfilter_dependencies = '<div class="admin-dependencies">';
  $pathfilter_dependencies .= t('Depends on: !dependencies', array('!dependencies' => (module_exists('pathfilter') ? t('@module (<span class="admin-enabled">enabled</span>)', array('@module' => 'Path Filter')) : t('@module (<span class="admin-disabled">disabled</span>)', array('@module' => 'Path Filter')))));
  $pathfilter_dependencies .= '</div>';

  $form['settings']['linkchecker_pathfilter_support'] = array(
    '#default_value' => variable_get('linkchecker_pathfilter_support', 0),
    '#type' => 'checkbox',
    '#title' => t('Pathfilter module support'),
    '#description' => t('Enable this checkbox for <a href="@pathfilter">pathfilter</a> module support. Pathfilter module need to be installed.', array('@pathfilter' => 'http://drupal.org/project/pathfilter')) . $pathfilter_dependencies,
    '#disabled' => module_exists('pathfilter') ? FALSE : TRUE,
  );

  $form['check'] = array(
    '#type' => 'fieldset',
    '#title' => t('Check settings'),
    '#description' => t('For simultaneous link checks it is highly recommended to install the <a href="@curl">cURL</a> library. This may be <strong>necessary</strong> on sites with very many links, but is every time good for link checking performance reasons.', array('@curl' => 'http://www.php.net/manual/en/book.curl.php')),
    '#collapsible' => FALSE,
  );
  $form['check']['linkchecker_check_useragent'] = array(
    '#type' => 'select',
    '#title' => t('User-Agent'),
    '#description' => t('Defines the user agent that will be used for checking links on remote sites. If someone blocks the standard Drupal user agent you can try with a more common browser.'),
    '#default_value' => variable_get('linkchecker_check_useragent', 'Drupal (+http://drupal.org/)'),
    '#options' => array(
      'Drupal (+http://drupal.org/)' => 'Drupal (+http://drupal.org/)',
      'Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1;)' => 'Windows XP / Internet Explorer 7.0',
      'Mozilla/5.0 (Windows; U; Windows NT 5.1; de; rv:1.9.0.5) Gecko/2008120122 Firefox/3.0.5' => 'Windows XP / Mozilla Firefox 3.0.5',
      'Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 6.0;)' => 'Windows Vista / Internet Explorer 7.0',
      'Mozilla/5.0 (Windows; U; Windows NT 6.0; de; rv:1.9.0.5) Gecko/2008120122 Firefox/3.0.5' => 'Windows Vista / Mozilla Firefox 3.0.5',
    ),
  );
  $form['check']['linkchecker_check_links_max'] = array(
    '#type' => 'select',
    '#title' => t('Check number of links per cron run'),
    '#description' => t('Defines the number of links that will be checked per cron run. The possible setting hardly depends on your PHP timeout value and remote servers speed and may increased if <a href="@curl">cURL</a> is installed and use simultaneous request checking. All cron jobs may fail, if this setting is too high!', array('@curl' => 'http://www.php.net/manual/en/book.curl.php')),
    '#default_value' => variable_get('linkchecker_check_links_max', 10),
    '#options' => drupal_map_assoc(array(5, 10, 25, 50, 100, 200, 250, 300, 350, 400, 450, 500, 750, 1000)),
  );
  $form['check']['linkchecker_check_links_interval'] = array(
    '#type' => 'select',
    '#title' => t('Check interval for links'),
    '#description' => t('This interval setting defines how often cron will re-check the status of links.'),
    '#default_value' => variable_get('linkchecker_check_links_interval', 2419200),
    '#options' => drupal_map_assoc(array(86400, 172800, 259200, 604800, 1209600, 2419200, 4838400), 'format_interval'),
  );
  $form['check']['linkchecker_cleanup_links_interval'] = array(
    '#type' => 'select',
    '#title' => t('Cleanup interval for unused links'),
    '#description' => t('This interval setting defines how often cron will clean up unused links.'),
    '#default_value' => variable_get('linkchecker_cleanup_links_interval', 604800),
    '#options' => drupal_map_assoc(array(86400, 172800, 259200, 604800, 1209600, 2419200, 4838400), 'format_interval'),
  );

  $form['error'] = array(
    '#type' => 'fieldset',
    '#title' => t('Error handling'),
    '#description' => t('Defines error handling and custom actions to be executed if specific HTTP requests are failing.'),
    '#collapsible' => FALSE,
  );
  $form['error']['linkchecker_action_status_code_301'] = array(
    '#title' => t('Update permanently moved links'),
    '#description' => t('If enabled, changed links providing a <em>Moved Permanently</em> (status code 301) are automatically updated. If used, it is recommended to use a value of <em>three</em> to make sure this is not only a temporarily change. This feature trust remote sites to provide a valid permanent redirect. Changes will be logged in <a href="@dblog">recent log entries</a>.', array('@dblog' => url('admin/reports/dblog'))),
    '#type' => 'select',
    '#default_value' => variable_get('linkchecker_action_status_code_301', 0),
    '#options' => array(
      0 => t('Disabled'),
      1 => t('After one failed check'),
      2 => t('After two failed checks'),
      3 => t('After three failed checks'),
      5 => t('After five failed checks'),
      10 => t('After ten failed checks'),
    ),
  );
  $form['error']['linkchecker_action_status_code_404'] = array(
    '#title' => t('Unpublish node on file not found error'),
    '#description' => t('If enabled, a node with one or more broken links (status code 404) will be unpublished and moved to moderation queue for review after the number of specified fails. If used, it is recommended to use a value of <em>three</em> to make sure this is not only a temporarily error.'),
    '#type' => 'select',
    '#default_value' => variable_get('linkchecker_action_status_code_404', 0),
    '#options' => array(
      0 => t('Disabled'),
      1 => t('After one file not found error'),
      2 => t('After two file not found errors'),
      3 => t('After three file not found errors'),
      5 => t('After five file not found errors'),
      10 => t('After ten file not found errors'),
    ),
  );
  $form['error']['linkchecker_ignore_response_codes'] = array(
    '#default_value' => variable_get('linkchecker_ignore_response_codes', "200\n304\n401\n403"),
    '#type' => 'textarea',
    '#title' => t("Don't treat these response codes as errors"),
    '#description' => t('One HTTP status code per line, e.g. 403.')
  );

  // Buttons are only required for testing and debugging reasons.
  $form['clear'] = array(
    '#type' => 'fieldset',
    '#title' => t('Clear link data'),
    '#description' => t('This action clears all link checker tables and/or and analyzes all node types, blocks and cck fields. Normally there is no need to press one of this buttons. Use this only for immediate cleanup tasks and to force a full re-check of all links.'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['clear']['linkchecker_analyze'] = array(
    '#type' => 'submit',
    '#value' => t('Analyze content for links'),
    '#submit' => array('linkchecker_analyze_links_submit'),
  );
  $form['clear']['linkchecker_clear_analyze'] = array(
    '#type' => 'submit',
    '#value' => t('Clear link data and analyze content for links'),
    '#submit' => array('linkchecker_clear_analyze_links_submit'),
  );

  $form['buttons']['submit'] = array('#type' => 'submit', '#value' => t('Save configuration') );
  $form['buttons']['reset'] = array('#type' => 'submit', '#value' => t('Reset to defaults') );

  return $form;
}

function linkchecker_admin_settings_form_validate($form, &$form_state) {
  $form_state['values']['linkchecker_ignore_response_codes'] = trim($form_state['values']['linkchecker_ignore_response_codes']);
  $ignore_response_codes = preg_split('/(\r\n?|\n)/', $form_state['values']['linkchecker_ignore_response_codes']);
  foreach ($ignore_response_codes as $ignore_response_code) {
    if (!_linkchecker_isvalid_response_code($ignore_response_code)) {
      form_set_error('linkchecker_ignore_response_codes', t('Invalid response code %code found.', array('%code' => $ignore_response_code)));
    }
  }
}

function linkchecker_admin_settings_form_submit($form, &$form_state) {
  // Exclude unnecessary elements.
  unset($form_state['values']['linkchecker_analyze'], $form_state['values']['linkchecker_clear_analyze']);

  // Node type selection changed?
  $additional_nodetypes_selected = array_diff($form_state['values']['linkchecker_scan_nodetypes'], variable_get('linkchecker_scan_nodetypes', array()));

  // Save form settings.
  system_settings_form_submit($form, $form_state);

  // Re-scan node types if new node types have been selected.
  // FIXME: or blocks needs to be scanned.
  if (!empty($additional_nodetypes_selected)) {
    linkchecker_analyze_links_submit();
  }
}

/**
 * Submit callback; Analyze all node types, boxes and cck fields.
 */
function linkchecker_analyze_links_submit() {
  // Start batch and analyze all nodes.
  $node_types = array_keys(array_filter(variable_get('linkchecker_scan_nodetypes', array())));
  if (!empty($node_types)) {
    batch_set(_linkchecker_batch_import_nodes($node_types));
    if (variable_get('linkchecker_scan_comments', 0)) {
      batch_set(_linkchecker_batch_import_comments($node_types));
    }
  }

  // FIXME: Batch title does not update on second batch_set(), http://drupal.org/node/368742
  if (variable_get('linkchecker_scan_blocks', 0)) {
    batch_set(_linkchecker_batch_import_boxes());
  }
}

/**
 * Submit callback; Clear link data and analyze all node types, boxes and cck fields.
 */
function linkchecker_clear_analyze_links_submit() {
  // TODO: D7 requires "DELETE FROM {table}" for sqllite.
  db_query("TRUNCATE TABLE {linkchecker_nodes}");
  db_query("TRUNCATE TABLE {linkchecker_comments}");
  db_query("TRUNCATE TABLE {linkchecker_boxes}");
  db_query("TRUNCATE TABLE {linkchecker_links}");

  // Start batch and analyze all nodes.
  $node_types = array_keys(array_filter(variable_get('linkchecker_scan_nodetypes', array())));
  if (!empty($node_types)) {
    batch_set(_linkchecker_batch_import_nodes($node_types));
    if (variable_get('linkchecker_scan_comments', 0)) {
      batch_set(_linkchecker_batch_import_comments($node_types));
    }
  }

  // FIXME: Batch title does not update on second batch_set(), http://drupal.org/node/368742
  if (variable_get('linkchecker_scan_blocks', 0)) {
    batch_set(_linkchecker_batch_import_boxes());
  }
}

/**
 * Menu callback for reporting.
 */
function linkchecker_admin_report() {

  $header = array(
    array('data' => t('URL'), 'field' => 'url', 'sort' => 'desc'),
    array('data' => t('Response'), 'field' => 'code', 'sort' => 'desc'),
    array('data' => t('Error'), 'field' => 'error'),
    array('data' => t('Operations')),
  );

  // FIXME: If a link has been corrected it is nevertheless shown until cleanup job ran.
  $ignore_response_codes = preg_split('/(\r\n?|\n)/', variable_get('linkchecker_ignore_response_codes', "200\n304\n401\n403"));
  $result = pager_query("SELECT * FROM {linkchecker_links} WHERE last_checked <> %d AND code NOT IN (" . db_placeholders($ignore_response_codes, 'int') . ")" . tablesort_sql($header), 50, 0, NULL, array_merge(array(0), $ignore_response_codes));

  $rows = array();
  while ($link = db_fetch_object($result)) {
    $links = array();

    // Show link to nodes having this broken link.
    $nodes = db_query('SELECT nid FROM {linkchecker_nodes} WHERE lid = %d', $link->lid);
    while ($node = db_fetch_object($nodes)) {
      $links[] = l(t('Edit node @node', array('@node' => $node->nid)), 'node/' . $node->nid . '/edit', array('query' => 'destination=admin/reports/linkchecker'));
    }

    // Show link to comments having this broken link.
    $comments = db_query('SELECT cid FROM {linkchecker_comments} WHERE lid = %d', $link->lid);
    while ($comment = db_fetch_object($comments)) {
      $links[] = l(t('Edit comment @comment', array('@comment' => $comment->cid)), 'comment/edit/' . $comment->cid, array('query' => 'destination=admin/reports/linkchecker'));
    }

    // Show link to blocks having this broken link.
    $boxes = db_query('SELECT bid FROM {linkchecker_boxes} WHERE lid = %d', $link->lid);
    while ($box = db_fetch_object($boxes)) {
      $links[] = l(t('Edit block @block', array('@block' => $box->bid)), 'admin/build/block/configure/block/' . $box->bid, array('query' => 'destination=admin/reports/linkchecker'));
    }

    // Create table data for output.
    $rows[] = array(
      l(_filter_url_trim($link->url, 40), $link->url),
      $link->code,
      $link->error,
      implode('<br/>', $links),
    );
  }

  if (empty($rows)) {
    $rows[] = array(array('data' => t('No broken links have been found.'), 'colspan' => '4'));
  }

  $output  = theme('table', $header, $rows);
  $output .= theme('pager', NULL, 3000, 0);
  return $output;
}

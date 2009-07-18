<?php
// $Id$

/**
 * @file
 * User page callbacks for the linkchecker module.
 */

/**
 * Menu callback for reporting.
 */
function linkchecker_admin_report() {

  $links_unchecked = db_result(db_query('SELECT count(lid) FROM {linkchecker_links} WHERE last_checked = %d', 0));
  if ($links_unchecked > 0) {
    $links_all = db_result(db_query('SELECT count(lid) FROM {linkchecker_links}'));
    drupal_set_message(format_plural($links_unchecked,
      'There is 1 unchecked link of about @links_all links in the database. Please be patient until all links have been checked via cron.',
      'There are @count unchecked links of about @links_all links in the database. Please be patient until all links have been checked via cron.',
      array('@links_all' => $links_all)), 'warning');
  }

  $header = array(
    array('data' => t('URL'), 'field' => 'url', 'sort' => 'desc'),
    array('data' => t('Response'), 'field' => 'code', 'sort' => 'desc'),
    array('data' => t('Error'), 'field' => 'error'),
    array('data' => t('Operations')),
  );

  $ignore_response_codes = preg_split('/(\r\n?|\n)/', variable_get('linkchecker_ignore_response_codes', "200\n304\n401\n403"));
  $links_report_sql = "SELECT ll.*
    FROM {linkchecker_links} ll
    INNER JOIN (
      SELECT DISTINCT lid FROM (
        SELECT DISTINCT lid FROM {linkchecker_boxes}
        UNION ALL
        SELECT DISTINCT lid FROM {linkchecker_comments}
        UNION ALL
        SELECT DISTINCT lid FROM {linkchecker_nodes}
      ) q1
    ) q2 ON q2.lid = ll.lid
    WHERE ll.last_checked <> %d AND ll.code NOT IN (" . db_placeholders($ignore_response_codes, 'int') . ")";
  $result = pager_query($links_report_sql . tablesort_sql($header), 50, 0, NULL, array_merge(array(0), $ignore_response_codes));

  // Evaluate permission once for performance reasons.
  $access_edit_link_settings = user_access('edit link settings');

  $rows = array();
  while ($link = db_fetch_object($result)) {
    $links = array();

    // Show links to link settings.
    if ($access_edit_link_settings) {
      $links[] = l(t('Edit link settings'), 'linkchecker/' . $link->lid . '/edit', array('query' => 'destination=admin/reports/linkchecker'));
    }

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

    // Show link to redirect this broken internal link.
    if (module_exists('path_redirect') && _linkchecker_is_internal_url($link)) {
      $links[] = l(t('Create redirection'), 'admin/build/path-redirect/add', array('query' => array('src' => $link->internal)));
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

function linkchecker_link_edit_form(&$form_state, $link) {

  $form['settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Settings'),
    '#collapsible' => FALSE,
    '#description' => t('The link <a href="@url">@url</a> has been checked lastly at @last_checked and failed @fail_count times.', array('@url' => $link['url'], '@fail_count' => $link['fail_count'], '@last_checked' => format_date($link['last_checked'])))
  );

  $form['settings']['lid'] = array('#type' => 'hidden', '#value' => $link['lid']);
  $form['settings']['url'] = array('#type' => 'hidden', '#value' => $link['url']);

  $form['settings']['method'] = array(
    '#type' => 'select',
    '#title' => t('Select request method'),
    '#default_value' => $link['method'],
    '#options' => array(
      'HEAD' => t('HEAD'),
      'GET' => t('GET'),
    ),
    '#description' => t('Select the request method used for link checks of this link. If you encounter issues like status code 500 errors with the HEAD request method you should try the GET request method before ignoring a link.'),
  );

  $form['settings']['status'] = array(
    '#default_value' => $link['status'],
    '#type' => 'checkbox',
    '#title' => t('Check link status'),
    '#description' => t("Disable this checkbox if you don't like to get informed any longer about this broken link. Use this setting only as the very last option if there is no other way to solve a failed link check."),
  );

  $form['buttons']['submit'] = array('#type' => 'submit', '#value' => t('Save configuration') );
  $form['buttons']['reset'] = array('#type' => 'submit', '#value' => t('Reset to defaults') );

  return $form;
}

function linkchecker_link_edit_form_submit($form, &$form_state) {
  if ($form_state['values']['method'] != $form['settings']['method']['#default_value']) {
    // Update settings and reset statistics for a quick re-check.
    db_query("UPDATE {linkchecker_links} SET method = '%s', fail_count = %d, last_checked = %d, status = %d WHERE lid = %d", $form_state['values']['method'], 0, 0, $form_state['values']['status'], $form_state['values']['lid']);
    drupal_set_message(t('The link settings for %url have been saved and the fail counter has been resetted.', array('%url' => $form_state['values']['url'])));
  }
  else {
    // Update setting only.
    db_query("UPDATE {linkchecker_links} SET method = '%s', status = %d WHERE lid = %d", $form_state['values']['method'], $form_state['values']['status'], $form_state['values']['lid']);
    drupal_set_message(t('The link settings for %url have been saved.', array('%url' => $form_state['values']['url'])));
  }
}

function _linkchecker_is_internal_url(&$link) {
  global $base_url;

  if (strpos($link->url, $base_url) === 0) {
    $link->internal = trim(substr($link->url, strlen($base_url)), " \t\r\n\0\\/");
    return TRUE;
  }
}

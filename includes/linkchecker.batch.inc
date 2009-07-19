<?php
// $Id$

/**
 * @file
 * Batch API callbacks for the linkchecker module.
 *
 * TODO: Test with 500.000+ nodes/comments/blocks and review memory consumption.
 */

/**
 * Batch: Scan nodes for links.
 */
function _linkchecker_batch_import_nodes($node_types = array()) {
  // Get all active {node}.nid's.
  $result = db_query('SELECT n.nid FROM {node} n WHERE n.status = %d AND n.type IN (' . db_placeholders($node_types, 'varchar') . ') ORDER BY n.nid', array_merge(array(1), $node_types));

  $operations = array();
  while ($row = db_fetch_array($result)) {
    $operations[] = array('_linkchecker_batch_node_import_op', array($row['nid']));
  }
  $batch = array(
    'file' => drupal_get_path('module', 'linkchecker') .'/includes/linkchecker.batch.inc',
    'finished' => '_linkchecker_batch_node_import_finished',
    'operations' => $operations,
    'title' => t('Scanning for links'),
  );

  return $batch;
}

/**
 * Batch operation: Scan ony by one node for links.
 */
function _linkchecker_batch_node_import_op($nid, &$context) {
  // Load the node and scan for links.
  $node = node_load($nid, NULL, TRUE);
  _linkchecker_add_node_links($node);

  // Store results for post-processing in the finished callback.
  $context['results'][] = $node->nid;
  $context['message'] = t('Node: @title', array('@title' => check_plain($node->title)));
}

function _linkchecker_batch_node_import_finished($success, $results, $operations) {
  if ($success) {
    $message = format_plural(count($results), 'One node has been scanned.', '@count nodes have been scanned.');
  }
  else {
    $message = t('Scanning for links in nodes have failed with an error.');
  }
  drupal_set_message($message);
}

/**
 * Batch: Scan comments for links.
 */
function _linkchecker_batch_import_comments($node_types = array()) {
  // Get all active {comments}.cid's.
  $result = db_query('SELECT c.cid FROM {comments} c INNER JOIN {node} n ON c.nid = n.nid WHERE c.status = %d AND n.status = %d AND n.type IN (' . db_placeholders($node_types, 'varchar') . ') ORDER BY c.cid', array_merge(array(COMMENT_PUBLISHED, 1), $node_types));

  $operations = array();
  while ($row = db_fetch_array($result)) {
    $operations[] = array('_linkchecker_batch_comments_import_op', array($row['cid']));
  }
  $batch = array(
    'file' => drupal_get_path('module', 'linkchecker') .'/includes/linkchecker.batch.inc',
    'finished' => '_linkchecker_batch_comments_import_finished',
    'operations' => $operations,
    'title' => t('Scanning for links'),
  );

  return $batch;
}

/**
 * Batch operation: Scan ony by one comment for links.
 */
function _linkchecker_batch_comments_import_op($cid, &$context) {
  // Load the comment and scan for links.
  $comment = _linkchecker_comment_load($cid);
  _linkchecker_add_comment_links($comment);

  // Store results for post-processing in the finished callback.
  $context['results'][] = $comment['cid'];
  $context['message'] = t('Comment: @title', array('@title' => check_plain($comment['subject'])));
}

function _linkchecker_batch_comments_import_finished($success, $results, $operations) {
  if ($success) {
    $message = format_plural(count($results), 'One comment has been scanned.', '@count comments have been scanned.');
  }
  else {
    $message = t('Scanning for links in comments have failed with an error.');
  }
  drupal_set_message($message);
}

/**
 * Batch: Scan blocks for links.
 */
function _linkchecker_batch_import_boxes() {
  // Get all active {boxes}.bid's.
  $result = db_query('SELECT b.bid FROM {boxes} b ORDER BY b.bid');

  $operations = array();
  while ($row = db_fetch_array($result)) {
    $operations[] = array('_linkchecker_batch_import_boxes_op', array($row['bid']));
  }
  $batch = array(
    'file' => drupal_get_path('module', 'linkchecker') .'/includes/linkchecker.batch.inc',
    'finished' => '_linkchecker_batch_box_import_finished',
    'operations' => $operations,
    'title' => t('Scanning for links'),
  );

  return $batch;
}

/**
 * Batch operation: Scan ony by one block for links.
 */
function _linkchecker_batch_import_boxes_op($bid, &$context) {
  // Load the box and scan for links.
  $box = block_box_get($bid);
  _linkchecker_add_box_links($box, $box['bid']);

  // Store some result for post-processing in the finished callback.
  $context['results'][] = $box['bid'];
  $context['message'] = t('Block: @title', array('@title' => check_plain($box['info'])));
}

function _linkchecker_batch_box_import_finished($success, $results, $operations) {
  if ($success) {
    $message = format_plural(count($results), 'One block has been scanned.', '@count blocks have been scanned.');
  }
  else {
    $message = t('Scanning for links in blocks have failed with an error.');
  }
  drupal_set_message($message);
}

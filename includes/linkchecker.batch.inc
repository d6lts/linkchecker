<?php
// $Id$

/**
 * @file
 * Batch API callbacks for the linkchecker module.
 */

/**
 * Batch: Load all nodes 100 by hundred.
 */
function _linkchecker_batch_import_nodes($node_types = array()) {
  // TODO: Add limitation setting and test with 500.000+ nodes and review memory consumption.
  $limit = 100;

  // Node types import count.
  $rows = db_result(db_query('SELECT COUNT(DISTINCT nid) FROM {node} WHERE status = %d AND type IN (' . db_placeholders($node_types, 'varchar') . ')', array_merge(array(1), $node_types)));

  $operations = array();
  for ($row = 0; $row<$rows; $row = $row+$limit) {
    $operations[] = array('_linkchecker_batch_node_import_op', array($row, $limit, $node_types));
  }
  $batch = array(
    'file' => drupal_get_path('module', 'linkchecker') .'/includes/linkchecker.batch.inc',
    'operations' => $operations,
    'finished' => '_linkchecker_batch_node_import_finished',
    'title' => t('Scanning nodes for links'),
  );

  return $batch;
}

/**
 * Batch operation: Load all nodes, 100 by hundred.
 */
function _linkchecker_batch_node_import_op($row, $limit, $node_types, &$context) {

  // Retrieve the next group of records.
  $result = db_query_range('SELECT nid FROM {node} WHERE status = %d AND type IN (' . db_placeholders($node_types, 'varchar') . ') ORDER BY nid ASC', array_merge(array(1), $node_types), $row, $limit);
  while ($res = db_fetch_array($result)) {
    // Load the node and scan for links.
    $node = node_load($res['nid'], NULL, TRUE);
    _linkchecker_add_node_links($node);

    // Store results for post-processing in the finished callback.
    // TODO: Test with 500.000+ nodes and review memory consumption.
    $context['results'][] = $node->nid;
  }
}

function _linkchecker_batch_node_import_finished($success, $results, $operations) {
  if ($success) {
    $message = format_plural(count($results), 'One node has been scanned.', '@count nodes have been scanned.');
  }
  else {
    $message = t('Finished importing nodes with an error.');
  }
  drupal_set_message($message);
}

/**
 * Batch: Load all comments 100 by hundred.
 */
function _linkchecker_batch_import_comments($node_types = array()) {
  // TODO: Add limitation setting and test with 500.000+ comments and review memory consumption.
  $limit = 100;

  // Comments import count.
  $rows = db_result(db_query('SELECT COUNT(DISTINCT cid) FROM {comments} c INNER JOIN {node} n ON c.nid = n.nid WHERE c.status = %d AND n.status = %d AND n.type IN (' . db_placeholders($node_types, 'varchar') . ')', array_merge(array(COMMENT_PUBLISHED, 1), $node_types)));

  $operations = array();
  for ($row = 0; $row<$rows; $row = $row+$limit) {
    $operations[] = array('_linkchecker_batch_comments_import_op', array($row, $limit, $node_types));
  }
  $batch = array(
    'file' => drupal_get_path('module', 'linkchecker') .'/includes/linkchecker.batch.inc',
    'operations' => $operations,
    'finished' => '_linkchecker_batch_comments_import_finished',
    'title' => t('Scanning comments for links'),
  );

  return $batch;
}

/**
 * Batch operation: Load all comments, 100 by hundred.
 */
function _linkchecker_batch_comments_import_op($row, $limit, $node_types, &$context) {

  // Retrieve the next group of records.
  $result = db_query_range('SELECT cid FROM {comments} c INNER JOIN {node} n ON c.nid = n.nid WHERE c.status = %d AND n.status = %d AND n.type IN (' . db_placeholders($node_types, 'varchar') . ') ORDER BY cid ASC', array_merge(array(COMMENT_PUBLISHED, 1), $node_types), $row, $limit);
  while ($res = db_fetch_array($result)) {
    // Load the comment and scan for links.
    $comment = _linkchecker_comment_load($res['cid']);
    _linkchecker_add_comment_links($comment);

    // Store results for post-processing in the finished callback.
    // TODO: Test with 500.000+ comments and review memory consumption.
    $context['results'][] = $comment['cid'];
  }
}

function _linkchecker_batch_comments_import_finished($success, $results, $operations) {
  if ($success) {
    $message = format_plural(count($results), 'One comment has been scanned.', '@count comments have been scanned.');
  }
  else {
    $message = t('Finished importing comments with an error.');
  }
  drupal_set_message($message);
}

/**
 * Batch: Load all boxes 100 by hundred.
 */
function _linkchecker_batch_import_boxes() {
  // TODO: Add limitation setting and test with 500.000+ boxes and review memory consumption.
  $limit = 100;

  // Block import.
  $operations = array();
  $rows = db_result(db_query('SELECT COUNT(DISTINCT bid) FROM {boxes}'));
  for ($row = 0; $row<$rows; $row = $row+$limit) {
    $operations[] = array('_linkchecker_batch_import_boxes_op', array($row, $limit));
  }
  $batch = array(
    'file' => drupal_get_path('module', 'linkchecker') .'/includes/linkchecker.batch.inc',
    'operations' => $operations,
    'finished' => '_linkchecker_batch_box_import_finished',
    'title' => t('Scanning blocks for links'),
  );

  return $batch;
}

/**
 * Batch operation: Load all boxes, 100 by hundred.
 */
function _linkchecker_batch_import_boxes_op($row, $limit, &$context) {
  // Retrieve the next group of bids.
  $result = db_query_range("SELECT bid FROM {boxes} ORDER BY bid ASC", $row, $limit);

  while ($res = db_fetch_array($result)) {
    // Load the box and scan for links.
    $box = block_box_get($res['bid']);
    _linkchecker_add_box_links($box, $box['bid']);

    // Store some result for post-processing in the finished callback.
    $context['results'][] = $box['bid'];
  }
}

function _linkchecker_batch_box_import_finished($success, $results, $operations) {
  if ($success) {
    $message = format_plural(count($results), 'One block has been scanned.', '@count blocks have been scanned.');
  }
  else {
    $message = t('Finished importing blocks with an error.');
  }
  drupal_set_message($message);
}

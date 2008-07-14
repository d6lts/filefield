<?php

/**
 * @file
 * FileField field hooks and callbacks.
 */

/**
 * Implementation of CCK's hook_field().
 */
function filefield_field_load($node, $field, &$items, $teaser, $page) {
  if (empty($items)) {
    return array();
  }
  foreach ($items as $delta => $item) {
    // Despite hook_content_is_empty(), CCK still doesn't filter out
    // empty items from $op = 'load', so we need to do that ourselves.
    if (empty($item['fid']) || !($file = field_file_load($item['fid']))) {
      unset($items[$delta]);
    }
    else {
      $items[$delta] = array_merge($item, $file);
    }
  }
  $items = array_values($items); // compact deltas
  return array($field['field_name'] => $items);
}

function filefield_field_insert($node, $field, &$items, $teaser, $page) {
  return filefield_field_update($node, $field, $items, $teaser, $page);
}

function filefield_field_update($node, $field, &$items, $teaser, $page) {
  foreach ($items as $delta => $item) {
    $items[$delta] = field_file_save($node, $item);
    // Remove items from the array if they have been deleted.
    if (empty($items[$delta])) unset($items[$delta]);
  }
  $items = array_values($items); // compact deltas
}

function filefield_field_delete_revision($node, $field, &$items, $teaser, $page) {
  foreach ($items as $delta => $item) {
    // For hook_file($op = 'references'), remember that this is being deleted.
    $item['field_name'] = $field['field_name'];
    if (field_file_delete($item)) unset($items[$delta]);
    $items = array_values($items); // compact deltas
  }
}


function filefield_field_delete($node, $field, &$items, $teaser, $page) {
  foreach ($items as $delta => $item) {
    // For hook_file($op = 'references'), remember that this is being deleted.
    $item['field_name'] = $field['field_name'];
    field_file_delete($item);
  }
}

function filefield_field_sanitize($node, $field, &$items, $teaser, $page) {
  foreach ($items as $delta => $item) {
    // Cleanup $items during node preview.
    if (empty($item['fid']) || !empty($item['delete'])) {
      unset($items[$delta]);
      continue;
    }
    // Load the complete file if a filepath is not available.
    if (!empty($item['fid']) && empty($item['filepath'])) {
      $items[$delta] = array_merge($item, field_file_load($item['fid']));
    }
    // Add nid so formatters can create a link to the node.
    $items[$delta]['nid'] = $node->nid;
  }
}


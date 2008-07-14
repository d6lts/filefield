<?php
// $Id$
/**
 * @file
 * FileField: Defines a CCK file field type.
 *
 * Uses content.module to store the fid and field specific metadata,
 * and Drupal's {files} table to store the actual file data.
 *
 * This file contains CCK formatter related functionality.
 */

/**
 * Theme function for the 'default' filefield formatter.
 */
function theme_filefield_formatter_filefield_default($element) {
  $file = $element['#item'];
  $field = content_fields($element['#field_name']);
  $output = theme('filefield', $file, $field);
  return $output;
}

/**
 * Theme function for any file that is managed by filefield.
 * It doesn't really format stuff by itself but rather redirects to other
 * formatters that are telling us they want to handle the concerned file.
 *
 * This function checks if the file may be shown and returns an empty string
 * if viewing the file is not allowed for any reason. If you need to display it
 * in any case, please use theme('filefield') instead.
 */
function theme_filefield($file, $field) {
  if (!filefield_view_access($field['field_name'])) {
    return '<!-- you do not have access to view this file -->view_access_denied';
  }
  if ($field['force_list']) {
    $file['list'] = 1; // always show the files if that option is enabled
  }
  if (empty($file['list'])) {
    return '';
  }
  return theme('filefield_file', $file);
}

/**
 * Theme function for any file that is managed by filefield.
 * It doesn't really format stuff by itself but rather redirects to other
 * formatters that are telling us they want to handle the concerned file.
 *
 * This function does not check if the file may be shown, it returns markup
 * in any case (except if the file doesn't exist at all). If you need to check
 * permissions, please use theme('filefield_guarded') instead.
 */
function theme_filefield_unguarded($file, $field) {
  if (empty($file['fid']) || !is_file($file['filepath'])) {
    return '';
  }
  drupal_add_css(drupal_get_path('module', 'filefield') .'/filefield.css');

  return '<div class="filefield-item filefield-item-empty"/>';
}


/**
 * Theme function for the 'generic' single file formatter.
 */
function theme_filefield_file($file) {
  $path = $file['filepath'];
  $url = file_create_url($path);
  $icon = theme('filefield_icon', $file);
  return '<div class="filefield-file">'. $icon . l($file['filename'], $url) .'</div>';
}

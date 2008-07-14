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
  return 'filefiedl' . theme('filefield', $file);
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
    return '';
  }
  if ($field['force_list']) {
    $file['list'] = 1; // always show the files if that option is enabled
  }
  if (empty($file['list'])) {
    return '';
  }
  return theme('filefield_unguarded', $file, $field);
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

  $file_formatter_info = filefield_formatter_for_file($file, $field);
  if (empty($file_formatter_info)) {
    return '<div class="filefield-item filefield-item-empty"/>';
  }

  foreach ($file_formatter_info['css'] as $css_path) {
    drupal_add_css($css_path);
  }
  $settings = isset($field['file_formatters'][$file_formatter_info['key']])
    ? $field['file_formatters'][$file_formatter_info['key']]
    : NULL;

  return '<div class="filefield-item">'.
    theme($file_formatter_info['theme'], (object)$file, $field, $settings)
    .'</div>';
}


/**
 * Theme function for the 'generic' single file formatter.
 */
function theme_filefield_file($file) {
  $path = $file['filepath'];
  $url = file_create_url($path);
  $icon = theme('filefield_icon', $file);
  $desc = $file['description'];
  return '<div class="filefield-file">'. $icon . l($desc, $url) .'</div>';
}

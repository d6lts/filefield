<?php

/**
 * @file 
 * 
 * FileField Widget Settings Hooks.
 */

function filefield_widget_settings_form($widget) {
  $form = array();
  $form['file_extensions'] = array(
    '#type' => 'textfield',
    '#title' => t('Permitted upload file extensions'),
    '#default_value' => is_string($widget['file_extensions']) ? $widget['file_extensions'] : 'txt',
    '#size' => 64,
    '#description' => t('Extensions a user can upload to this field. Separate extensions with a space and do not include the leading dot. Leaving this blank will allow users to upload a file with any extension.'),
    '#weight' => 1,
  );
  $form['path_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Path settings'),
    '#collapsible' => true,
    '#collapsed' => true,
    '#weight' => 6,
  );

  $form['path_settings']['file_path'] = array(
    '#type' => 'textfield',
    '#title' => t('File path'),
    '#default_value' => is_string($widget['file_path']) ? $widget['file_path'] : '',
    '#description' => t('Optional subdirectory within the "%dir" directory where files will be stored. Do not include trailing slash.', array('%dir' => variable_get('file_directory_path', 'files'))),
    '#element_validate' => array('_filefield_widget_settings_file_path_validate'),
    '#suffix' => theme('token_help', 'user'),
  );

  $form['max_filesize'] = array(
    '#type' => 'fieldset',
    '#title' => t('File size restrictions'),
    '#description' => t('Limits for the size of files that a user can upload. Note that these settings only apply to newly uploaded files, whereas existing files are not affected.'),
    '#weight' => 6,
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['max_filesize']['max_filesize_per_file'] = array(
    '#type' => 'textfield',
    '#title' => t('Maximum upload size per file'),
    '#default_value' => is_string($widget['max_filesize_per_file'])
                        ? $widget['max_filesize_per_file']
                         : '',
    '#description' => t('Specify the size limit that applies to each file separately. Enter a value like "512" (bytes), "80K" (kilobytes) or "50M" (megabytes) in order to restrict the allowed file size. If you leave this this empty the file sizes will be limited only by PHP\'s maximum post and file upload sizes.'),
    '#element_validate' => array('_filefield_widget_settings_max_filesize_per_file_validate'),
  );
  $form['max_filesize']['max_filesize_per_node'] = array(
    '#type' => 'textfield',
    '#title' => t('Maximum upload size per node'),
    '#default_value' => is_string($widget['max_filesize_per_node'])
                        ? $widget['max_filesize_per_node']
                        : '',
    '#description' => t('Specify the total size limit for all files in field on a given node. Enter a value like "512" (bytes), "80K" (kilobytes) or "50M" (megabytes) in order to restrict the total size of a node. Leave this empty if there should be no size restriction.'),
    '#element_validate' => array('_filefield_widget_settings_max_filesize_per_node_validate'),
  );
  return $form;
}

function filefield_widget_settings_save($widget) {
  return array(
    'file_extensions', 'file_path', 'max_filesize_per_file',
    'max_filesize_per_node', 'file_widgets'
  );
}

function _filefield_widget_settings_file_path_validate($element, &$form_state) {
  // Strip slashes from the beginning and end of $widget['file_path']
  $form_state['values']['file_path'] = trim($form_state['values']['file_path'], '\\/');
}

function _filefield_widget_settings_max_filesize_per_file_validate($element, &$form_state) {
  if (empty($form_state['values']['max_filesize_per_file'])) {
    return; // Empty means no size restrictions, so don't throw an error.
  }
  else if (!is_numeric(parse_size($form_state['values']['max_filesize_per_file']))) {
    form_error($element, t('The "Maximum file size for each file" option must contain a valid value. You can either leave the text field empty or enter a string like "512" (bytes), "80K" (kilobytes) or "50M" (megabytes).'));
  }
}

function _filefield_widget_settings_max_filesize_per_node_validate($element, &$form_state) {
  if (empty($form_state['values']['max_filesize_per_node'])) {
    return; // Empty means no size restrictions, so don't throw an error.
  }
  else if (!is_numeric(parse_size($form_state['values']['max_filesize_per_node']))) {
    form_error($element, t('The "Maximum file size per node" option must contain a valid value. You can either leave the text field empty or enter a string like "512" (bytes), "80K" (kilobytes) or "50M" (megabytes).'));
  }
}



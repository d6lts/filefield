<?php
// $Id$
/**
 * @file
 * FileField: Defines a CCK file field type.
 *
 * Uses content.module to store the fid and field specific metadata,
 * and Drupal's {files} table to store the actual file data.
 *
 * This file contains CCK widget related functionality.
 */

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

/**
 * FileField widget element callbacks.
 */
function filefield_widget_value($element, $edit = FALSE) {
  if (!$edit) { 
    $file =  field_file_load($element['#default_value']['fid']);
    $item = $element['#default_value'];
  }
  else {
    $file = field_file_load($edit['fid']);
    $item = array(
      'fid' => $edit['fid'],
      'description' => $edit['description'],
      'list' => empty($edit['list']),
      'data' => '',
    );
  }
  $item = array_merge($item, $file);

  // if this widget is another type and leaning on filefield to do the dirty work....
  // pass it back home.
  $function = $element['#type'] .'_widget_value';
  if (function_exists($function)) {
    $item = array_merge($item, $function($element, $edit));
  }
  return $item;
}

function filefield_widget_process($element, $edit, &$form_state, $form) {
  $item  = empty($edit) ? $element['#default_value'] : $edit;
  $item = array_merge($item, field_file_load($item['fid']));
  
  $field = content_fields($element['#field_name'], $element['#type_name']);
  
  $element['fid'] = array('#type' => 'hidden', '#default_value' =>  $item['fid']);

  if ($item['fid'] != 0) {
    $element['preview'] = array('#type' => 'markup', '#value' => theme($element['#type'] .'_preview', $item));  
  }
  $element['description'] = array(
    '#type' => 'textfield', 
    '#title' => t('Description'),
    '#default_value' => $item['description'],
  );
  
  $element['list'] = array(
    '#type' => 'checkbox',
    '#title' => t('List'),
    '#default_value' => !empty($item['list']),
    '#attributes' => array('class' => 'filefield-list'),
  );  
  if ($field['force_list']) $element['list']['#type'] = 'hidden';

  $element['data'] = array(
    '#type' => 'hidden', 
    '#value' => '',
  );

  if ($item['fid'] != 0) {
    $element['remove_btn'] = array(
      '#name' => $element['#field_name'] .'_'. $element['#delta'] .'_remove_btn',
      '#type' => 'submit', 
      '#value' => t('Remove'),
      '#submit' => array('filefield_widget_submit_remove_btn'),
      '#field_name' => $element['#field_name'],
      '#delta' => $element['#delta'],
    );
  }

  if ($item['fid'] == 0) {
    $element['description']['#type'] = 'hidden';

    $element['upload'] = array(
      '#name' => 'files['. $element['#field_name'] .'_'. $element['#delta'] .']',
      '#type' => 'file',
      '#title' => t('New Upload'),
    );

    $element['upload_btn'] = array(
      '#name' => $element['#field_name'] .'_'. $element['#delta'] .'_upload_btn',
      '#type' => 'submit', 
      '#value' => t('Upload'),
      '#submit' => array('filefield_widget_submit_upload_btn'),
      '#field_name' => $element['#field_name'],
      '#delta' => $element['#delta'],
    );
  }

  // call widget type specific callbacks.
  $function = $element['#type'] .'_widget_process';
  if (function_exists($function)) {
    $element =  $function($element, $edit, $form_state, $form);
  }
  return $element;
}

function filefield_widget_after_build($element) {
  $element['#value'] = array(   
    'fid' => $element['fid']['#value'],   
    'description' => $element['description']['#value'],   
    'list' => $element['list']['#value'],   
    'data' => $element['data']['#value'],   
  );    

  // if this widget is another type and leaning on filefield to do the dirty work....
  // pass it back home.
  $function = $element['#type'] .'_widget_after_build';
  if (function_exists($function)) {
    $element = $function($element, $edit, $form_state, $form);
  }
 
  return $element;    
}   

   

function filefield_widget_submit_remove_btn($form, &$form_state) {
  dsm ('imagefield_widget_submit_remove_btn');
  //dsm($form);
  $field_name = $form_state['clicked_button']['#field_name'];
  $delta = $form_state['clicked_button']['#delta'];

  dsm($form_state['values'][$field_name][$delta]);
  $form_state['values'][$field_name][$delta] = array(
    'fid' => 0,
    'alt' => '',
    'filepath' => '',
    'filename' => '',
    'title' => '',
    'data' => '',
  ); 
  $form_state['rebuild'] = true;
}

function filefield_widget_submit_upload_btn($form, &$form_state) {
  //dsm('imagefield_widget_submit_upload_btn');
  //dsm($form);
  //dsm($form_state);
  $field_name = $form_state['clicked_button']['#field_name'];
  $delta = $form_state['clicked_button']['#delta'];

  $field = content_fields($field_name);

  $widget_file_path = $field['widget']['file_path'];
  if (module_exists('token')) {
    global $user;
    $widget_file_path = token_replace($widget_file_path, 'user', $user);
  }

  $complete_file_path = file_directory_path() .'/'. $widget_file_path;
  $upload_name = $field_name .'_'. $delta;
  if (!field_file_check_directory($complete_file_path, FILE_CREATE_DIRECTORY)) {
    watchdog('imagefield', 'The upload directory %directory for the file field %field (content type %type) could not be created or is not accessible. A newly uploaded file could not be saved in this directory as a consequence, and the upload was canceled.', array('%directory' => $widget_file_path, '%field' => $field_name, '%type' => $field['type_name']));
    form_set_error($upload_name, t('The file could not be uploaded.'));
    return $file;
  }

  $validators = _filefield_filefield_validators($field, $field['widget'], $form_state['values'][$field_name]);

  if (!$file = field_file_save_upload($upload_name, $validators, $complete_file_path)) {
    watchdog('imagefield', 'The file upload failed. %upload', array('%upload' => $upload_name));
    form_set_error($upload_name, t('The Image upload failed.'));
    return;
  }
  //dsm($file);
  $form_state['values'][$field_name][$delta]['fid'] = $file['fid'];
  $form_state['rebuild'] = true;
}

function _filefield_widget_validate($element, &$form_state) {
  //dsm('imagefield_validate');
  //dsm($element);
}


/**
 * FormAPI theme function. Theme the output of an image field.
 */
function theme_filefield_widget($element) {
  return $element['#children'];
}

function theme_filefield_widget_preview($item) {
  return '<div class="filefield-preview clear-block">'.
           '<div class="filename">'. theme('filefield_file', $item) .'</div>'.
           '<div class="filesize">'. format_size($item['filesize']) .'</div>'.
           '<div class="filemime">'. $item['filemime'] .'</div>'.
         '</div>';
}

function theme_filefield_widget_item($element) {
  return '<div class="filefield-row clear-block">'.
              '<div class="filefield-filename clear-block">'. drupal_render($element['preview']) . '</div>'.
              '<div class="fielfield-edit clear-block">'.
                '<div class="filefield-list">'. drupal_render($element['list']) . '</div>' .
                '<div class="filefield-description">'. drupal_render($element['description']) . '</div>' .
                '<div class="filefield-stuff">'. drupal_render($element) .'</div>'.
              '</div>'.
         '</div>';
}




/**
 * Implementation of hook_filefield_validators():
 * Upload restrictions for file size, file extension and supported file widgets.
 * Implemented as private function instead of as a real hook, because we want
 * to make an exception so that these requirements appear first in any list.
 */
function _filefield_filefield_validators($field, $widget, $existing_files) {
  $validators = array();

  // Thanks to the php.ini restrictions, there is always a maximum file size.
  // Therefore we can rely on at least one restriction always being in force.
  $max_filesize = _filefield_maximum_filesize($field, $widget, $existing_files);
  $filesize_message = ($max_filesize > 0)
    ? t('Maximum file size: !size.', array('!size' => format_size($max_filesize)))
    : t('The allowed maximum file size total has been exceeded.');

  $validators['file_validate_size'] = array(
    'validator arguments' => array($max_filesize),
    'requirement message' => $filesize_message,
    'upload possible' => ($max_filesize > 0),
  );

  if (!empty($widget['file_extensions'])) {
    $validators['file_validate_extensions'] = array(
      'validator arguments' => array($widget['file_extensions']),
      'requirement message' => t('Allowed extensions: %ext.', array(
        '%ext' => $widget['file_extensions'],
      )),
    );
  }
  return $validators;
}



/**
 * Get the maximum file size that is allowed for a new upload.
 *
 * @return
 *   -1 for "no more files allowed", or any positive value as the number
 *   of bytes that may still be uploaded. A result of 0 ("unlimited") will
 *   never happen because of PHP's upload limits.)
 */
function _filefield_maximum_filesize($field, $widget, $existing_files) {
  // Calculate the maximum file size - the least of all returned values.
  $max_filesize = FALSE;
  $restrictions = module_invoke_all(
    'filefield_filesize_restrictions', $field, $widget, $existing_files
  );
  foreach ($restrictions as $value) {
    if ($max_filesize === FALSE || $value < $max_filesize) {
      $max_filesize = $value;
    }
  }

  // Return -1 if any restriction value was not a positive number.
  if ($max_filesize === FALSE || $max_filesize <= 0) {
    return -1;
  }
  return $max_filesize;
}

/**
 * Implementation of hook_filefield_filesize_restrictions():
 * Specify how large a newly uploaded file may be, in bytes.
 * (The smallest size of all hook implementations will be applied in the end).
 */
function filefield_filefield_filesize_restrictions($field, $widget, $existing_files) {
  $filesize_restrictions = array(file_upload_max_size());

  // Maximum file size for each file separately.
  if (!empty($widget['max_filesize_per_file'])) {
    $filesize_restrictions[] = parse_size($widget['max_filesize_per_file']);
  }

  // Maximum file size for all files in the node (for this field).
  if (!empty($widget['max_filesize_per_node'])) {
    $allowed_total_size = parse_size($widget['max_filesize_per_node']);
    $total_size = 0;

    foreach ($existing_files as $delta => $file) {
      if (!empty($file->filesize)) {
        $total_size += $file->filesize;
      }
    }
    if (!empty($total_size)) {
      $filesize_restrictions[] = $allowed_total_size - $total_size;
    }
  }
  return $filesize_restrictions;
}





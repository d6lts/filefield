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

function filefield_widget_value($element, $edit = FALSE) {
  //dsm('imagefield_widget_value');
  //dsm($edit);
  
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
  return array_merge($item, $file);
}

function filefield_widget_process($element, $edit, &$form_state, $form) {
  //dsm('imagefield_widget_process');
  //dsm($element);
  //dsm($form_state);
  $element['#theme'] = 'filefield_row';
  $file = $element['#value'];
  $delta = $element['#delta'];
  $field_name = $element['#field_name'];

  
  $element['fid'] = array('#type' => 'hidden', '#value' =>  $file['fid']);

  if ($file['fid'] != 0) {
    $element['preview'] = array('#type' => 'markup', '#value' => theme('filefield_file', $file));  
    if (empty($file['filepath'])) {
      //dsm($file);
    }
  }
  $element['description'] = array(
    '#type' => 'textfield', 
    '#title' => t('Description'),
    '#default_value' => $file['description'],
  );
  
  $element['list'] = array(
    '#type' => 'checkbox',
    '#title' => t('List'),
    '#default_value' => $file['list'],
    '#attributes' => array('class' => 'filefield-list'),
  );  

  $element['data'] = array(
    '#type' => 'hidden', 
    '#value' => '',
  );

  if ($file['fid'] != 0) {
    $element['remove_btn'] = array(
      '#name' => $field_name .'_'. $delta .'_remove_btn',
      '#type' => 'submit', 
      '#value' => t('Remove'),
      '#submit' => array('filefield_widget_submit_remove_btn'),
      '#field_name' => $field_name,
      '#delta' => $delta,
    );
    return $element;
  }

  if ($file['fid'] == 0) {
    $element['description']['#type'] = 'hidden';

    $element['upload'] = array(
      '#name' => 'files['. $field_name .'_'. $delta .']',
      '#type' => 'file',
      '#title' => t('New Upload'),
    );

    $element['upload_btn'] = array(
      '#name' => $field_name .'_'. $delta .'_upload_btn',
      '#type' => 'submit', 
      '#value' => t('Upload'),
      '#submit' => array('filefield_widget_submit_upload_btn'),
      '#field_name' => $field_name,
      '#delta' => $delta,
    );
  }
  return $element;
}

function filefield_widget_submit_remove_btn($form, &$form_state) {
  //dsm ('imagefield_widget_submit_remove_btn');
  //dsm($form);
  //dsm($form_state);
  $field_name = $form_state['clicked_button']['#field_name'];
  $delta = $form_state['clicked_button']['#delta'];
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

function filefield_widget_submit_reuse_btn($form, &$form_state) {
  //dsm('imagefield_widget_submit_reuse_btn');
  //dsm($form);
  //dsm($form_state);

  $field_name = $form_state['clicked_button']['#field_name'];
  $delta = $form_state['clicked_button']['#delta'];

  $reuse =  trim($form_state['values'][$field_name][$delta]['reuse']);
  if (empty($reuse))  return;

  // if numeric assume we got an fid and try to load it.
  if (is_numeric($reuse) && $file = field_file_load($reuse)) {
    $form_state['values'][$field_name][$delta]['fid'] = $file['fid'];
    $form_state['rebuild'] = true;
  } 
  else {
    // load fid from path..
    $result = db_query("SELECT fid FROM {files} WHERE filepath='%s'", $reuse);
    if ($fid = db_result($result)) {
      $form_state['values'][$field_name][$delta]['fid'] = $fid;
      $form_state['rebuild'] = true;
    }
  }
}

function filefield_widget_after_build($element) {
  //dsm('imagefield_widget_after_build');
  //dsm($element);
 
  $element['#value'] = array(
    'fid' => $element['fid']['#value'],
    'description' => $element['description']['#value'],
    'list' => $element['list']['#value'],
    'data' => $element['data']['#value'],
  );
  return $element;
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

function theme_filefield_row($element) {
  return '<div class="filefield-row clear-block">'.
              '<div class="filefield-info clear-block">'. 
                  '<div class="filefield-filename clear-block">'. drupal_render($element['preview']) . '</div>'.
                  //'<div class="filefield-filesize">filesize: 32Kb</div>'.
                  //'<div class="filefield-filemime">mimetype: text/patch</div>'.
              '</div>' .
              '<div class="fielfield-edit clear-block"'.
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



/**
 * Shared AHAH callback for uploads and deletions. It just differs in a few
 * unimportant details (what happens to the file, and which form is used as
 * a replacement) so these details are taken care of by a form callback.
 */
function filefield_js($field_name, $type_name, $delta, $form_callback) {
  $field = content_fields($field_name, $type_name);

  if (empty($field) || empty($_POST['form_build_id'])) {
    // Invalid request.
    print drupal_to_js(array('data' => ''));
    exit;
  }

  // Build the new form.
  $form_state = array('submitted' => FALSE);
  $form_build_id = $_POST['form_build_id'];
  $form = form_get_cache($form_build_id, $form_state);

  if (!$form) {
    // Invalid form_build_id.
    print drupal_to_js(array('data' => ''));
    exit;
  }
  // form_get_cache() doesn't yield the original $form_state,
  // but form_builder() does. Needed for retrieving the file array.
  $built_form = $form;
  $built_form_state = $form_state;
  $built_form += array('#post' => $_POST);
  $built_form = form_builder($_POST['form_id'], $built_form, $built_form_state);

  // Clean ids, so that the same element doesn't get a different element id
  // when rendered once more further down.
  form_clean_id(NULL, TRUE);

  // Perform the action for this AHAH callback.
  $form_callback($built_form, $built_form_state, $field, $delta);

  // Ask CCK for the replacement form element. Going through CCK gets us
  // the benefit of nice stuff like '#required' merged in correctly.
  module_load_include('inc', 'content', 'includes/content.node_form');
  $field_element = content_field_form($form, $built_form_state, $field, $delta);
  $delta_element = $field_element[$field_name][0]; // there's only one element in there

  // Add the new element at the right place in the form.
  if (module_exists('fieldgroup') && ($group_name = _fieldgroup_field_get_group($type_name, $field_name))) {
    $form[$group_name][$field_name][$delta] = $delta_element;
  }
  else {
    $form[$field_name][$delta] = $delta_element;
  }

  // Write the (unbuilt, updated) form back to the form cache.
  form_set_cache($form_build_id, $form, $form_state);

  // Render the form for output.
  $form += array(
    '#post' => $_POST,
    '#programmed' => FALSE,
  );
  drupal_alter('form', $form, array(), 'filefield_js');
  $form_state = array('submitted' => FALSE);
  $form = form_builder('filefield_js', $form, $form_state);
  $field_form = empty($group_name) ? $form[$field_name] : $form[$group_name][$field_name];

  // We add a div around the new content to tell AHAH to let this fade in.
  $field_form[$delta]['#prefix'] = '<div class="ahah-new-content">'. (isset($field_form[$delta]['#prefix']) ? $field_form[$delta]['#prefix'] : '');
  $field_form[$delta]['#suffix'] = (isset($field_form[$delta]['#suffix']) ? $field_form[$delta]['#suffix'] : '') .'</div>';

  $output = theme('status_messages') . drupal_render($field_form[$delta]);

  // AHAH is not being nice to us and doesn't know the "other" button (that is,
  // either "Upload" or "Delete") yet. Which in turn causes it not to attach
  // AHAH behaviours after replacing the element. So we need to tell it first.
  $javascript = drupal_add_js(NULL, NULL);
  if (isset($javascript['setting'])) {
    $output .= '<script type="text/javascript">jQuery.extend(Drupal.settings, '. drupal_to_js(call_user_func_array('array_merge_recursive', $javascript['setting'])) .');</script>';
  }

  // For some reason, file uploads don't like drupal_json() with its manual
  // setting of the text/javascript HTTP header. So use this one instead.
  print drupal_to_js(array('status' => TRUE, 'data' => $output));
  exit;
}

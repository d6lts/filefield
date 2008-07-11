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
 * Implementation of hook_widget() - the one in filefield.module
 * is just there to include this one on demand.
 */
function filefield_widget(&$form, &$form_state, $field, $items, $delta = 0) {
  // JavaScript might reload the form element but not the CSS that has been
  // defined inside the process hook of mimetype specific widgets. As we don't
  // know which widget will show up, just include all of their CSS files.
  $file_widget_info = _filefield_file_widget_info($field['widget']);
  _filefield_add_css($file_widget_info);
  drupal_add_css(drupal_get_path('module', 'filefield') .'/filefield.css');

  if (!empty($items[$delta]) && $file = field_file_load($items[$delta]['fid'])) {
    $file = array_merge($items[$delta], $file);
    return filefield_file_edit_form($form, $form_state, $field, $delta, $file);
  }
  return filefield_file_upload_form($form, $form_state, $field, $delta, $items);
}

/**
 * The filefield widget for not (yet) existing files.
 */
function filefield_file_upload_form(&$form, &$form_state, $field, $delta, $items) {
  $form['#attributes']['enctype'] = 'multipart/form-data';

  // Include JavaScript for client-side file validation.
  drupal_add_js(drupal_get_path('module', 'filefield') .'/filefield.js');

  $field_name_css = str_replace('_', '-', $field['field_name']);
  $id = 'filefield-'. $field_name_css .'-'. $delta .'-form';

  $replaced_file = (isset($items[$delta]) && isset($items[$delta]['replaced_file']))
                    ? $items[$delta]['replaced_file'] : NULL;

  $requirements = _filefield_upload_requirements($field, $field['widget'], $items);

  $widget = array(
    '#type'   => 'filefield_file_upload',
    '#field' => $field,
    '#delta' => $delta,
    '#replaced_file' => $replaced_file,
    '#prefix' => '<div id="'. $id .'" class="filefield-file-form"><div class="filefield-file-upload">',
    '#suffix' => '</div></div>',
    '#upload_requirements' => $requirements,
  );

  if ($requirements['upload possible']) {
    // Buttons inside custom form elements are not registered by the Forms API,
    // so we make the "Upload" button a regular child element and not a part
    // of the filefield_file_upload widget.
    $widget[$field['field_name'] .'_'. $delta .'_upload'] = array(
      '#name' => $field['field_name'] .'_'. $delta .'_upload',
      '#type' => 'submit',
      '#value' => t('Upload'),
      '#submit' => array('filefield_file_upload_submit'), // without JavaScript
      '#ahah' => array( // with JavaScript
        'path' => 'filefield/js/upload/'. $field['field_name'] .'/'. $field['type_name'] .'/'. $delta,
        'wrapper' => $id,
        'method' => 'replace',
        'effect' => 'fade',
      ),
      '#weight' => 10,
      '#field' => $field,
      '#delta' => $delta,
    );
  }
  return $widget;
}

/**
 * The 'process' callback for 'filefield_file_upload' form elements.
 * Called after defining the form and while building it, transforms the
 * barebone element array into a file selection widget.
 */
function filefield_file_upload_process($element, $edit, &$form_state, $form) {
  // Before the element user gets to do his validation, make sure we do ours.
  array_unshift($element['#element_validate'], 'filefield_file_upload_validate');

  $field = $element['#field'];
  $field_name = $field['field_name'];
  $upload_name = $field_name .'_'. $element['#delta'];

  $requirements = $element['#upload_requirements'];

  // Construct the upload description out of user supplied text,
  // maximum upload file size, and (optionally) allowed extensions.
  if ($requirements['upload possible'] == FALSE) {
    $element[$upload_name] = array(
      '#type' => 'markup',
      '#value' => t('!errors No new files can be uploaded anymore.', array(
        '!errors' => implode(' ', $requirements['messages']),
      )),
    );
    return $element;
  }

  // Make a list out of the messages if there are too many restrictions.
  // Looks better than a concatenated sequence of sentences.
  $upload_description = (count($requirements['messages']) > 2)
    ? '<ul><li>'. implode('</li><li>', $requirements['messages']) .'</li></ul>'
    : implode(' ', $requirements['messages']);

  $element[$upload_name] = array(
    '#type' => 'file',
    '#title' => t('Attach new file'),
    '#description' => $upload_description,
    '#weight' => -1,
    // Emulate how FAPI normalizes the _FILES array since this won't go through form_builder
    '#name' => 'files['. $upload_name .']',
  );

  // User 1 may even upload files with extensions that are not allowed.
  // (At least, that's how core's file_validate_extensions() thinks about it.)
  // So only add the JavaScript extension check for other users.
  global $user;
  if ($user->uid != 1) {
    $element[$upload_name]['#attributes'] = array(
      'accept' => str_replace(' ', '|', trim($field['widget']['file_extensions']))
    );
  }

  return $element;
}

/**
 * Theme function for the file upload container element.
 */
function theme_filefield_file_upload($element) {
  return theme('filefield_container_item', $element);
}

/**
 * Value callback for 'filefield_upload' form elements.
 * Uploads and validates a file if one has been specified,
 * and returns the fid of that file as result value.
 */
function filefield_file_upload_value($element, $edit = FALSE) {
  return empty($element['#value'])
    ? array('fid' => 0, 'replaced_file' => $element['#replaced_file'])
    : $element['#value'];
}

/**
 * The 'validate' callback for 'duration_combo' form elements.
 * Called after values are assigned, before form validate and submit are called.
 */
function filefield_file_upload_validate($element, &$form_state) {
  if (!empty($element['#required']) && empty($element['#value']['fid'])) {
    form_error($element, t('You need to upload a file to the %field field.', array(
      '%field' => $element['#field']['widget']['label']
    )));
  }
}

/**
 * Submit callback for the "Upload" button next to each file upload field.
 */
function filefield_file_upload_submit($form, &$form_state) {
  $field = $form_state['clicked_button']['#field'];
  $delta = $form_state['clicked_button']['#delta'];
  _filefield_file_upload($form_state, $field, $delta);

  // Rebuild the form with the new uploaded-file state (hopefully).
  node_form_submit_build_node($form, $form_state);
}

/**
 * Form callback for the "Upload" button with JavaScript enabled,
 * invoked by filefield_js(). Uploads a file to the given field and delta.
 */
function filefield_file_upload_js(&$form, &$form_state, $field, $delta) {
  _filefield_file_upload($form_state, $field, $delta);
}

/**
 * Upload a file to the given field and delta (or try to, at least), and
 * update the corresponding part of the form state with the new file data.
 */
function _filefield_file_upload(&$form_state, $field, $delta) {
  $field_name = $field['field_name'];

  if (module_exists('token')) {
    global $user;
    $widget_file_path = token_replace($field['widget']['file_path'], 'user', $user);
  }
  else {
    $widget_file_path = $field['widget']['file_path'];
  }

  // Let modules provide their own validators.
  $validators = _filefield_upload_validators(
    $field, $field['widget'], $form_state['values'][$field_name]
  );
  $upload_name = $field_name .'_'. $delta;
  $complete_file_path = file_directory_path() .'/'. $widget_file_path;

  $file = &$form_state['values'][$field_name][$delta];
  $replaced_file = $file['replaced_file'];

  if (!filefield_check_directory($widget_file_path, $upload_name)) {
    watchdog('file', 'The upload directory %directory for the file field %field (content type %type) could not be created or is not accessible. A newly uploaded file could not be saved in this directory as a consequence, and the upload was canceled.', array('%directory' => $widget_file_path, '%field' => $field_name, '%type' => $field['type_name']));
    $file = array('fid' => 0, 'replaced_file' => $replaced_file);
    return $file;
  }
  if (!$file = field_file_save_upload($upload_name, $validators, $complete_file_path)) {
    watchdog('file', 'The file %file could not be saved as addition to the file field %field (content type %type). This can be a consequence of the file failing validation, or if it can\'t be moved to the file directory, or whatever reason the file framework comes up with. No further information is available to the filefield module, but if you\'re lucky then that function left one or more hints in the log as well (directly before this log entry).', array('%file' => $complete_file_path, '%field' => $field_name, '%type' => $field['type_name']));
    $file = array('fid' => 0, 'replaced_file' => $replaced_file);
    return $file;
  }

  $file_default_properties = array(
    'list' => 1,
    'description' => $file['filename'],
  );
  $file = array_merge($file_default_properties, $file);
  $file['replaced_file'] = $replaced_file;
  return $file;
}

/**
 * Retrieve a list of file validator functions (and arguments) that can be
 * passed to file_save_upload() (or field_file_save_upload()) as is.
 */
function _filefield_upload_validators($field, $widget, $items) {
  $validator_info = _filefield_validator_info($field, $widget, $items);
  $validators = array();

  foreach ($validator_info as $validator_function => $info) {
    $validators[$validator_function] = $info['validator arguments'];
  }
  return $validators;
}

/**
 * Retrieve a list of upload requirement strings for the various upload
 * restrictions that this module and possible extensions provide.
 */
function _filefield_upload_requirements($field, $widget, $items) {
  $validator_info = _filefield_validator_info($field, $widget, $items);
  $messages = array();
  $errors = array();

  foreach ($validator_info as $validator_function => $info) {
    $messages[] = $info['requirement message'];

    if (isset($info['upload possible']) && $info['upload possible'] == FALSE) {
      $errors[] = $info['requirement message'];
    }
  }
  return array(
    'messages' => empty($errors) ? $messages : $errors,
    'upload possible' => empty($errors),
  );
}

/**
 * Retrieve an array of file validators and their associated requirement
 * messages (placing filefield's own validators first in the result array).
 */
function _filefield_validator_info($field, $widget, $items) {
  // Clean out empty items, so that they're not taken into account by
  // implementations of hook_filefield_validators() - they're irrelevant here.
  // Also, objectify items - because we're leaving the pure filefield realm.
  $existing_files = array();
  foreach ($items as $delta => $item) {
    if (is_array($item) && !empty($item['fid'])) {
      $existing_files[] = (object) $item;
    }
  }
  return array_merge(
    _filefield_filefield_validators($field, $widget, $existing_files),
    module_invoke_all('filefield_validators', $field, $widget, $existing_files)
  );
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
  $max_filesize = _filefield_maximum_filesize(
    $field, $widget, $existing_files
  );
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

  $supported_file_widgets = _filefield_supported_file_widgets($widget);
  if (!empty($supported_file_widgets)) {
    $validators['filefield_validate_file_widget_support'] = array(
      'validator arguments' => array($field, $widget, $supported_file_widgets),
      'requirement message' => t('Uploads are restricted to the following categories: !widgets.', array(
        '!widgets' => implode(', ', $supported_file_widgets),
      )),
    );
  }
  return $validators;
}

/**
 * Check that a file is supported by at least one of the widgets that are
 * enabled for the field instance in question.
 *
 * @return
 *   An array. If the file is not allowed, it will contain an error message.
 */
function filefield_validate_file_widget_support($file, $field, $field_widget, $supported_file_widgets) {
  $errors = array();

  // No widgets at all means the widget settings db entry does not exist,
  // so we fall back to "accept this file and use the generic edit widget".
  if (empty($field_widget['file_widgets'])) {
    return $errors;
  }
  // In the common case, we only accept a file if an enabled widget
  // wants to handle it.
  $edit_widget_info = filefield_widget_for_file($file, $field, $field_widget);
  if (empty($edit_widget_info)) {
    $errors[] = t('Uploaded files are restricted to the following categories: !widgets.', array(
      '!widgets' => implode(', ', $supported_file_widgets),
    ));
  }
  return $errors;
}

/**
 * If not all file types might be handled by the enabled set of file widgets,
 * return an array of widget titles specifying which ones are allowed for the
 * given field. If a widget is enabled which handles all files, return an empty array.
 */
function _filefield_supported_file_widgets($field_widget) {
  if (empty($field_widget['file_widgets'])) {
    return array();
  }
  $titles = array();
  $file_widget_info = _filefield_file_widget_info($field_widget);

  foreach ($file_widget_info as $widget_name => $info) {
    if (!$info['enabled']) {
      continue;
    }
    if ($info['suitability callback'] === TRUE) {
      // Handles all kinds of files, no need for requiring any other widget.
      return array();
    }
    $titles[] = $info['title'];
  }
  return $titles;
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
 * Create the file directory relative to the 'files' dir recursively for every
 * directory in the path.
 *
 * @param $directory
 *   The directory path under files to check, such as 'photo/path/here'.
 * @param $form_item
 *   An optional string containing the name of a form item that any errors
 *   will be attached to. (See field_file_check_directory() for more details.)
 */
function filefield_check_directory($directory, $form_item = NULL) {
  $directory = field_file_strip_path($directory);

  foreach (explode('/', $directory) as $dir) {
    $dirs[] = $dir;
    $path = file_create_path(implode($dirs, '/'));
    if (!field_file_check_directory($path, FILE_CREATE_DIRECTORY, $form_item)) {
      watchdog('filefield', t('FileField failed to create directory (%d) at (%p).', array('%d' => $directory, '%p' => $path)), WATCHDOG_ERROR);
      return FALSE;
    }
  }
  return TRUE;
}


/**
 * The filefield widget for previously uploaded files.
 */
function filefield_file_edit_form(&$form, &$form_state, $field, $delta, $file) {
  $field_name_css = str_replace('_', '-', $field['field_name']);
  $id = 'filefield-'. $field_name_css .'-'. $delta .'-form';

  $classes = array(
    'filefield-'. $field_name_css .'-form',
    'filefield-file-form',
  );
  $widget = array(
    '#type' => 'filefield_file_edit',
    '#default_value' => $file,
    '#field' => $field,
    '#prefix' => '<div id="'. $id .'" class="'. implode(' ', $classes) .'"><div class="filefield-file-edit">',
    '#suffix' => '</div></div>',
  );

  // Buttons inside custom form elements are not registered by the Forms API,
  // so we make the "Delete" button a regular child element and not a part
  // of the filefield_file_upload widget.
  $widget['flags'] = array(
    '#type' => 'markup',
    '#value' => '',
    '#prefix' => '<div class="filefield-file-edit-flags">',
    '#suffix' => '</div>',
  );
  $widget['flags'][$field['field_name'] .'_'. $delta .'_delete'] = array(
    '#name' => $field['field_name'] .'_'. $delta .'_delete',
    '#type' => 'submit',
    '#value' => t('Delete'),
    '#submit' => array('filefield_file_edit_delete_submit'), // without JavaScript
    '#ahah' => array( // with JavaScript
      'path' => 'filefield/js/delete/'. $field['field_name'] .'/'. $field['type_name'] .'/'. $delta,
      'wrapper' => $id,
      'method' => 'replace',
      'effect' => 'fade',
    ),
    '#field' => $field,
    '#delta' => $delta,
    '#file' => $file,
  );
  // Only show the list checkbox if files are not forced to be listed.
  if (!$field['force_list']) {
    $widget['flags']['list'] = array(
      '#type' => 'checkbox',
      '#title' => t('List'),
      '#default_value' => $file['list'],
    );
  }

  $edit_widget_info = filefield_widget_for_file($file, $field, $field['widget']);
  $widget['edit'] = array(
    '#type' => empty($edit_widget_info)
                ? 'filefield_generic_edit' // as last-resort fallback
                : $edit_widget_info['form element'],
    '#field' => $field,
    '#delta' => $delta,
    '#file' => (object) $file,
    '#default_value' => $file,
    '#prefix' => '<div class="filefield-file-edit-widget">',
    '#suffix' => '</div>',
  );

  return $widget;
}

/**
 * Theme function for the file edit container element.
 */
function theme_filefield_file_edit($element) {
  return theme('filefield_container_item', $element);
}

/**
 * Custom value callback for file edit widgets, so that we don't need to rely
 * on a tree structure but can assemble the file to our likings.
 */
function filefield_file_edit_value($element, $edit = FALSE) {
  $file = $element['#default_value'];

  if (!is_array($edit)) {
    return $file;
  }

  // Form API is being mean to us and doesn't include the 'list' value
  // for checkboxes if the checkbox is disabled (0), only if it's enabled (1).
  // That means we need to go the extra mile.
  $list = isset($element['flags']['list'])
          ? !empty($edit['flags']['list'])
          : $file['list'];

  $file_fixed_properties = array(
    'list'      => $list,
    'delete'    => 0,
    'fid'       => $file['fid'],
    'uid'       => $file['uid'],
    'status'    => $file['status'],
    'filename'  => $file['filename'],
    'filepath'  => $file['filepath'],
    'filemime'  => $file['filemime'],
    'filesize'  => $file['filesize'],
    'timestamp' => $file['timestamp'],
  );

  if (is_array($edit['edit'])) {
    $file = array_merge($file, $edit['edit']);
  }
  $file = array_merge($file, $file_fixed_properties);

  // Also merge in other values that might come from other child form elements,
  // like the '_weight' property that CCK adds to this field.
  unset($edit['flags']);
  unset($edit['edit']);
  $file = array_merge($edit, $file);

  return $file;
}

/**
 * Submit callback for the "Delete" button next to each file item.
 */
function filefield_file_edit_delete_submit($form, &$form_state) {
  $field = $form_state['clicked_button']['#field'];
  $delta = $form_state['clicked_button']['#delta'];
  filefield_file_edit_delete($form_state, $field, $delta);

  // Rebuild the form with the new deleted-file state.
  node_form_submit_build_node($form, $form_state);
}

/**
 * Form callback for the "Delete" button with JavaScript enabled,
 * invoked by filefield_js(). Marks the file in the given field and delta
 * as deleted, or deletes it right away (depending on the context).
 */
function filefield_file_edit_delete_js(&$form, &$form_state, $field, $delta) {
  filefield_file_edit_delete($form_state, $field, $delta);
}

/**
 * Update the form state so that the file for the given field and delta
 * is marked as deleted.
 */
function filefield_file_edit_delete(&$form_state, $field, $delta) {
  $field_name = $field['field_name'];
  $file = &$form_state['values'][$field_name][$delta];

  if (isset($file['status']) && $file['status'] == FILE_STATUS_PERMANENT) {
    $file['delete'] = 1;
    $file = array(
      'fid' => 0,
      'replaced_file' => $file,
    );
  }
  else { // temporary file, get rid of it before it's even saved
    $empty_file = array(
      'fid' => 0,
      'replaced_file' => $file['replaced_file'], // remember permanent files from before
    );
    field_file_delete($file);
    $file = $empty_file;
  }
  return $file;
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

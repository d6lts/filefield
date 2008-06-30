<?php
// $Id$
/**
 * @file
 * FileField: Defines a CCK file field type.
 *
 * Uses content.module to store the fid and field specific metadata,
 * and Drupal's {files} table to store the actual file data.
 *
 * This file contains common theme functions.
 */

/**
 * Theme function for a file formatter / file widget settings table.
 */
function theme_filefield_draggable_settings_table($element) {
  $settings_type = $element['#settings_type']; // 'widgets' or 'formatters'?
  $table_id = 'filefield-file-'. $settings_type .'-table';
  $order_class = 'filefield-file-'. $settings_type .'-weight';

  $title = isset($element['#title']) ? $element['#title'] : '';
  unset($element['#title']); // the header is used instead of the regular label

  $required = !empty($element['#required'])
    ? '<span class="form-required" title="'. t('This field is required.') .'">*</span>'
    : '';

  $title_column = t('@title: !required', array(
    '@title' => $title, '!required' => $required,
  ));
  $header = array($title_column, t('Order'));
  $rows = array();

  foreach (element_children($element) as $key) {
    $element[$key]['weight']['#attributes']['class'] = $order_class;
    $delta_element = drupal_render($element[$key]['weight']);
    $row = array(drupal_render($element[$key]), $delta_element);
    $rows[] = array('data' => $row, 'class' => 'draggable');
  }
  $output = theme('table', $header, $rows, array('id' => $table_id));
  $output = theme('form_element', $element, $output);

  drupal_add_tabledrag($table_id, 'order', 'sibling', $order_class);

  return $output;
}

/**
 * Render either an upload or edit container item so that the children elements
 * always appear inside a nice table, whatever $field['multiple'] might be.
 */
function theme_filefield_container_item($element) {
  $field = $element['#field'];
  $children = !empty($element['#children']) ? $element['#children'] : '';
  $children = '<div class="ahah-new-content">'. $children .'</div>';

  // CCK renders a nice table for multiple-value fields, that's just fine as is.
  if ($field['multiple']) {
    return $children;
  }
  // If the field is single-value, we still want to have a table, for the looks.
  $header = array();
  $rows = array(array($children));
  $attributes = array('class' => 'filefield-file-container-table');
  $table = theme('table', $header, $rows, $attributes);
  return theme('form_element', $element, $table);
}

/**
 * Return an image with an appropriate icon for the given file.
 * Remember to pass a file object and not an array.
 */
function theme_filefield_icon($file) {
  if (is_object($file)) {
    $file = (array) $file;
  }
  $dashed_mime = check_plain(strtr($file['filemime'], array('/' => '-')));

  if ($icon_url = _filefield_icon_url($file)) {
    $icon = '<img class="field-icon-'. $dashed_mime .'" src="'. $icon_url .'" />';
  }
  return '<div class="filefield-icon field-icon-'. $dashed_mime .'">'. $icon .'</div>';
}

function _filefield_icon_url($file) {
  global $base_url;
  $theme = variable_get('filefield_icon_theme', 'protocons');

  if ($iconpath = _filefield_icon_path($file, $theme)) {
    return $base_url .'/'. $iconpath;
  }
  return FALSE;
}

function _filefield_icon_path($file, $theme = 'protocons') {
  // If there's an icon matching the exact mimetype, go for it.
  $dashed_mime = strtr($file['filemime'], array('/' => '-'));
  if ($iconpath = _filefield_create_icon_path($dashed_mime, $theme)) {
    return $iconpath;
  }
  // For a couple of mimetypes, we can "manually" tell a generic icon.
  if ($generic_name = _filefield_generic_icon_map($file)) {
    if ($iconpath = _filefield_create_icon_path($generic_name, $theme)) {
      return $iconpath;
    }
  }
  // Use generic icons for each category that provides such icons.
  foreach (array('audio', 'image', 'text', 'video') as $category) {
    if (strpos($file['filemime'], $category .'/') === 0) {
      if ($iconpath = _filefield_create_icon_path($category .'-x-generic', $theme)) {
        return $iconpath;
      }
    }
  }
  // Try application-octet-stream as last fallback.
  if ($iconpath = _filefield_create_icon_path('application-octet-stream', $theme)) {
    return $iconpath;
  }
  // Sorry, no icon can be found...
  return FALSE;
}

function _filefield_create_icon_path($iconname, $theme = 'protocons') {
  $iconpath = drupal_get_path('module', 'filefield')
    .'/icons/'. $theme .'/16x16/mimetypes/'. $iconname .'.png';
  if (file_exists($iconpath)) {
    return $iconpath;
  }
  return FALSE;
}

function _filefield_generic_icon_map($file) {
  switch ($file['filemime']) {
    // Word document types.
    case 'application/msword':
    case 'application/vnd.ms-word.document.macroEnabled.12':
    case 'application/vnd.oasis.opendocument.text':
    case 'application/vnd.oasis.opendocument.text-template':
    case 'application/vnd.oasis.opendocument.text-master':
    case 'application/vnd.oasis.opendocument.text-web':
    case 'application/vnd.openxmlformats-officedocument.wordprocessingml.document':
    case 'application/vnd.stardivision.writer':
    case 'application/vnd.sun.xml.writer':
    case 'application/vnd.sun.xml.writer.template':
    case 'application/vnd.sun.xml.writer.global':
    case 'application/vnd.wordperfect':
    case 'application/x-abiword':
    case 'application/x-applix-word':
    case 'application/x-kword':
    case 'application/x-kword-crypt':
      return 'x-office-document';

    // Spreadsheet document types.
    case 'application/vnd.ms-excel':
    case 'application/vnd.ms-excel.sheet.macroEnabled.12':
    case 'application/vnd.oasis.opendocument.spreadsheet':
    case 'application/vnd.oasis.opendocument.spreadsheet-template':
    case 'application/vnd.openxmlformats-officedocument.spreadsheetml.sheet':
    case 'application/vnd.stardivision.calc':
    case 'application/vnd.sun.xml.calc':
    case 'application/vnd.sun.xml.calc.template':
    case 'application/vnd.lotus-1-2-3':
    case 'application/x-applix-spreadsheet':
    case 'application/x-gnumeric':
    case 'application/x-kspread':
    case 'application/x-kspread-crypt':
      return 'x-office-spreadsheet';

    // Presentation document types.
    case 'application/vnd.ms-powerpoint':
    case 'application/vnd.ms-powerpoint.presentation.macroEnabled.12':
    case 'application/vnd.oasis.opendocument.presentation':
    case 'application/vnd.oasis.opendocument.presentation-template':
    case 'application/vnd.openxmlformats-officedocument.presentationml.presentation':
    case 'application/vnd.stardivision.impress':
    case 'application/vnd.sun.xml.impress':
    case 'application/vnd.sun.xml.impress.template':
    case 'application/x-kpresenter':
      return 'x-office-presentation';

    // Compressed archive types.
    case 'application/zip':
    case 'application/x-zip':
    case 'application/stuffit':
    case 'application/x-stuffit':
    case 'application/x-7z-compressed':
    case 'application/x-ace':
    case 'application/x-arj':
    case 'application/x-bzip':
    case 'application/x-bzip-compressed-tar':
    case 'application/x-compress':
    case 'application/x-compressed-tar':
    case 'application/x-cpio-compressed':
    case 'application/x-deb':
    case 'application/x-gzip':
    case 'application/x-java-archive':
    case 'application/x-lha':
    case 'application/x-lhz':
    case 'application/x-lzop':
    case 'application/x-rar':
    case 'application/x-rpm':
    case 'application/x-tzo':
    case 'application/x-tar':
    case 'application/x-tarz':
    case 'application/x-tgz':
      return 'package-x-generic';

    // Script file types.
    case 'application/ecmascript':
    case 'application/javascript':
    case 'application/mathematica':
    case 'application/vnd.mozilla.xul+xml':
    case 'application/x-asp':
    case 'application/x-awk':
    case 'application/x-cgi':
    case 'application/x-csh':
    case 'application/x-m4':
    case 'application/x-perl':
    case 'application/x-php':
    case 'application/x-ruby':
    case 'application/x-shellscript':
    case 'text/vnd.wap.wmlscript':
    case 'text/x-emacs-lisp':
    case 'text/x-haskell':
    case 'text/x-literate-haskell':
    case 'text/x-lua':
    case 'text/x-makefile':
    case 'text/x-matlab':
    case 'text/x-python':
    case 'text/x-sql':
    case 'text/x-tcl':
      return 'text-x-script';

    // HTML aliases.
    case 'application/xhtml+xml':
      return 'text-html';

    // Executable types.
    case 'application/x-macbinary':
    case 'application/x-ms-dos-executable':
    case 'application/x-pef-executable':
      return 'application-x-executable';

    default:
      return FALSE;
  }
}

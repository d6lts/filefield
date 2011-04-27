<?php
/**
 * @file
 * Returns a file icon based on its MIME type.
 */

class filefield_handler_field_icon extends views_handler_field {
  function construct() {
    parent::construct();
    $this->additional_fields['fid'] = 'fid';
    $this->additional_fields['filename'] = 'filename';
    $this->additional_fields['filesize'] = 'filesize';
  }

  function render($values) {
    $value = $this->get_value($values);
    $pseudo_file = array(
      'fid' => $this->get_value($values, 'fid'),
      'filemime' => $value,
      'filename' => $this->get_value($values, 'filename'),
      'filesize' => $this->get_value($values, 'filesize'),
    );
    return theme('filefield_icon', $pseudo_file);
  }
}

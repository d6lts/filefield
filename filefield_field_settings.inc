<?php

function filefield_field_settings_form($field) {
      $form = array();
      $form['force_list'] = array(
        '#type' => 'checkbox',
        '#title' => t('Always list files'),
        '#default_value' => isset($field['force_list']) ? $field['force_list'] : 0,
        '#description' => t('If enabled, the "List" checkbox will be hidden and files are always shown. Otherwise, the user can choose for each file whether it should be listed or not.'),
      );
      return $form;
}

function filefield_field_settings_validate($field) {
}

function filefield_field_settings_save($field) {
  return array('force_list', 'file_formatters');
}

function filefield_field_settings_database_columns($field) {
  return array(
    'fid' => array('type' => 'int', 'not null' => FALSE),
    'description' => array('type' => 'varchar', 'length' => 255, 'not null' => FALSE, 'sortable' => TRUE),
    'list' => array('type' => 'int', 'size' => 'tiny', 'not null' => FALSE),
    'data' => array('type' => 'text', 'serialize' => true),
  );
}

function filefield_field_settings_views_data($field) {
  $data = content_views_field_views_data($field);
  $db_info = content_database_info($field);
  $table_alias = content_views_tablename($field);

  // Set our own field handler so that we can hook the file formatter
  // configuration table into the options form.

  // By defining the relationship, we already have a "Has file" filter
  // plus all the filters that Views already provides for files.
  // No need for having a filter by ourselves.
  unset($data[$table_alias][$field['field_name'] .'_fid']['filter']);

  // Add a relationship for related file.
  $data[$table_alias][$field['field_name'] .'_fid']['relationship'] = array(
    'base' => 'files',
    'field' => $db_info['columns']['fid']['column'],
    'handler' => 'views_handler_relationship',
  );
  return $data;
}



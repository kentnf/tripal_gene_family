<?php

/**
 *
 */
function tripal_gene_family_delete ($form, $form_state) {

  // Set the breadcrumbs. 
  $breadcrumb = tripal_gene_family_breadcrumb();
  $breadcrumb[] = l('Delete gene family', 'admin/tripal/extension/tripal_gene_family/delete');
  drupal_set_breadcrumb($breadcrumb);

  // get gene family cvterm
  $family_options = tripal_gene_family_get_families();

  // generate form
  $form['family'] = array(
    '#type' => 'select',
    '#title' => t('Select Family'),
    '#options' => $family_options,
    '#description' => t('All the members under the selected family will be deleted.'),
    '#required' => TRUE,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Delete',
  );

  return $form;
}

/**
 * Implement hook_validate()
 */
function tripal_gene_family_delete_validate ($form, $form_state) {

  // check if the family has subfamily 

}

/**
 * Implement hook_submit()
 */
function tripal_gene_family_delete_submit ($form, $form_state) {

  $cvterm_id = $form_state['values']['family'];

  // delete featureprop 
  chado_delete_record('featureprop', array('type_id' => $cvterm_id));

  // delete cvterm
  chado_delete_record('cvterm', array('cvterm_id' => $cvterm_id));
 
  drupal_set_message(t('Family has been deleted.'), 'status');
}


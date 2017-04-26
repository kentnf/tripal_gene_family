<?php 

/**
 *
 */
function tripal_gene_family_update($form, $form_state) {

  // Set the breadcrumbs. 
  $breadcrumb = tripal_gene_family_breadcrumb();
  $breadcrumb[] = l('Update gene family', 'admin/tripal/extension/tripal_gene_family/update');
  drupal_set_breadcrumb($breadcrumb);

  // get gene family cvterm
  $first_option = array('0' => 'Top Level');
  $family_options = tripal_gene_family_get_families();
  $family_options = $first_option + $family_options;

  // generate form
  $form['family_id'] = array(
    '#type' => 'select',
    '#title' => t('Select Parent Family'),
    '#options' => $family_options,
    '#description' => t('Select Top Level if there is no parent of this family.'),
    '#required' => TRUE,
  );

  $form['family_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Family Name'),
    '#description' => t('Only alphanumeric characters and "_" are allowed. Example: abc_123.'),
    '#required' => TRUE,
  );

  $form['family_desc'] = array(
    '#type' => 'textarea',
    '#title' => t('Family Description'),
  );

  $form['feature_list'] = array(
    '#type' => 'textarea',  
    '#title' => t('Feature List'), 
    '#description' => t('One feature per line'),
    '#required' => TRUE, 
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Add',
  );

  return $form;
}

/**
 *Implement hook_validate()
 */
function tripal_gene_family_update_validate($form, &$form_state) {

  // check family name
  $family_name = $form_state['values']['family_name'];
  $values = array(
    'name' => $family_name,
    'cv_id' => array(
      'name' => 'tripal_gene_family',
    ),
  );
  $result = chado_select_record('cvterm', array('cvterm_id'), $values);

  if (sizeof($result) > 0) {
    form_set_error('family_name', t('The family %family_name already exist.', array('%family_name' => $family_name)));
  }

  // check feature list
  $feature_list = $form_state['values']['feature_list'];
  $feature_list = trim($feature_list);
  $features = explode("\n", $feature_list);
  $feature_ids = array(); 

  $sql = "select feature_id from chado.feature where uniquename = :uniquename";
  foreach ($features as $feature) {
    $feature = trim($feature);
    $result_id = db_query($sql, array(':uniquename' => $feature))->fetchAll();
    if (sizeof($result_id) > 1) {
      form_set_error('feature_list', t('Duplicate feature: %unique_name', array('%unique_name' => $feature)));
    }
    if (sizeof($result_id) == 0 ) {
      form_set_error('feature_list', t('Can not find feature: %unique_name', array('%unique_name' => $feature)));
    } 
    if (sizeof($result_id) == 1) { 
      $feature_ids[$result_id[0]->feature_id] = $feature;
    }
  }

  $form_state['values']['features'] = $feature_ids;
}

/**
 *Implement hook_submit()
 */
function tripal_gene_family_update_submit($form, $form_state) {

  // update cvterm
  $family_name = $form_state['values']['family_name'];
  $family_desc = $form_state['values']['family_desc'];
  $cvterm = tripal_insert_cvterm(array(
    'name' => $family_name,
    'definition' => $family_desc,
    'cv_name' => 'tripal_gene_family',
    'db_name' => 'tripal_gene_family',
  ));

  // delete feature according to cvterm id
  chado_delete_record('cvterm', array('cvterm_id' => $cvterm_id));


  /**
   * delete cvterm and add new
  if ($cvterm) {
    $cvterm_id = $cvterm->cvterm_id;
    // add featureprop
    $num_feature = 0;
    $features = $form_state['values']['features'];
    foreach ($features as $feature_id => $uniquename) {
      $values = array(
        'feature_id' => $feature_id,
        'type_id' => $cvterm_id,
        'rank' => 0,
      );
      $result = chado_insert_record('featureprop', $values);
      if (!$result) {
        drupal_set_message(t('Unable to assign %feature to %family.', array('%feature'=>$uniquename, '%family'=>$family_name)), 'error');
      }
      $num_feature++;

      drupal_set_message(t('Assign %num_feature feature(s) to %family.', array('%num_feature'=>$num_feature, '%family'=>$family_name)), 'status');
    }
  } 
  else {
    drupal_set_message(t('Unable to add cvterm %family_name.', array('%family_name'=>$family_name)), 'error');
  }
  */
}


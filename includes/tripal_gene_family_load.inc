<?php 

/**
 * @param array header
 *
 * @param array of 1st row
 *
 * @return HTML of table
 */
function tripal_gene_family_table_example ($header, $first_row) {

  $html = '<table><tr>';
  foreach ($header as $h) {
    $html.= '<th>' . $h . '</th>';
  }

  $html.= '</tr><tr>';
  foreach ($first_row as $r) {
    $html.= '<td>' . $r . '</td>';
  }

  $html.= '</tr></table>';
  return $html;
}

/**
 *
 */
function tripal_gene_family_load($form, $form_state) {

  // Set the breadcrumbs. 
  $breadcrumb = tripal_gene_family_breadcrumb();
  $breadcrumb[] = l('Load gene family', 'admin/tripal/extension/tripal_gene_family/load');
  drupal_set_breadcrumb($breadcrumb);

  // get gene family cvterm
  $first_option = array('0' => 'Top Level');
  $family_options = tripal_gene_family_get_families();
  $family_options = $first_option + $family_options;

  $head_data = array('Parent Name*','Family Name*', 'Family Description');
  $rows_data = array('TF','AP2', 'AP2 Desc');
  $t1 = tripal_gene_family_table_example($head_data, $rows_data);

  $head_data = array('Feature Name*','Family Name ID*');
  $rows_data = array('Cla00001','AP2');
  $t2 = tripal_gene_family_table_example($head_data, $rows_data);

  $form['description'] = array(
    '#markup' => t('This page is for loading families and gene classification table into 
       database. All the table should be tab-delimit format <b>without title</b> before 
       loading to database. Here, we just give an example of each table. <br> If families
       or gene classification records already exist in database, they will be updated instead 
       of insert.
       <br>' 
   ));

  // generate form
  // format : parent -- family -- desc

  // FEATURE SUMMARY REPORT
  $form['family'] = array(
     '#type' => 'fieldset',
     '#title' => t('Load Family Table'),
  );

  $form['family']['example'] = array(
    '#markup' => t('<b>Example of family table:</b> <br>' . $t1)
  );

  $form['family']['family_table'] = array(
    '#type' => 'textfield',
    '#title' => t('Family Table (tab delimit format)'),
    '#description' => t('/var/www/html/icugi3rd/sites/default/files/tripal_TF_rule.txt'),
  );

  $form['family']['submit1'] = array(
    '#type' => 'submit',
    '#value' => 'Load family',
  );

  // feature name -- family name
  $form['classify'] = array(
     '#type' => 'fieldset',
     '#title' => t('Load Gene Classification Table'),
  ); 

  $form['classify']['example'] = array(
    '#markup' => t('<b>Example of gene classification table:</b> <br>' . $t2)
  );

  $form['classify']['classify_table'] = array(
    '#type' => 'textfield',
    '#title' => t('Gene Classification Table (tab delimit format)'),
    '#description' => t('/var/www/html/icugi3rd/sites/default/files/tripal_PI_tf.txt'),
  );

  $form['classify']['submit2'] = array(
    '#type' => 'submit',
    '#value' => 'Load classification',
  );

  return $form;
}

/**
 *Implement hook_validate()
 */
function tripal_gene_family_load_validate($form, &$form_state) {

  // check files
  if ($form_state['values']['op'] == $form_state['values']['submit1']) {
    if (empty($form_state['values']['family_table'])) {
      form_set_error('family_table', t('The Field Family Table is required.'));
    }
 
    if (!is_readable($form_state['values']['family_table'])) {
      form_set_error('family_table', t('Can not Could not open the file %file. Check that file exists and that permissions are correct.', array('%file' => $form_state['values']['family_table'])));
    }
  } 
  elseif ($form_state['values']['op'] == $form_state['values']['submit2']) {
    if (empty($form_state['values']['classify_table'])) {
      form_set_error('classify_table', t('The Field Classify Table is required.'));
    }

    if (!is_readable($form_state['values']['classify_table'])) {
      form_set_error('classify_table', t('Can not Could not open the file %file. Check that file exists and that permissions are correct.', array('%file' => $form_state['values']['classify_table'])));
    }

  } 
  else {
    form_set_error('classify_table', 'debug');
  }
}

/**
 *Implement hook_submit()
 */
function tripal_gene_family_load_submit($form, $form_state) {

  // submit tripal_jop to process the family_table and classify_table
  global $user;

  if ($form_state['values']['op'] == $form_state['values']['submit1']) {
    $job_args = array(
      $form_state['values']['family_table'],
    );
    
    tripal_add_job("Load Family Table", 'tripal_gene_family',
      'tripal_gene_family_family_table_loader', $job_args, $user->uid);
  } 
  else {
    $job_args = array(
      $form_state['values']['classify_table'],
    );
    
    tripal_add_job("Load Classify Table", 'tripal_gene_family',
      'tripal_gene_family_classify_table_loader', $job_args, $user->uid);
  }
}

/**
 * check family table 
 * @param path of family table 
 * @return array of families
 */
function tripal_gene_family_family_table_check($family_table) {

  // check and load family_table to array
  $parent_cache = array(); // save the parent family 
  $family_cache = array(); // save the family   
  $fh = fopen($family_table, 'r');
  while($line = fgetcsv($fh, 0, "\t")) {
    if (sizeof($line) < 2) {
      print "The column number of family_table is not correct.\n"; 
      exit; 
    }
    $parent = $line[0];
    $family = $line[1];
    $description = '';
    if (!empty($line[2])) {
      $description = $line[2];
    }

    $values = array(
      'name' => $parent,
      'cv_id' => array(
        'name' => 'tripal_gene_family',
      ),
    );

    if (strcmp($parent, '0')) {
      $parent_id = chado_select_record('cvterm', array('cvterm_id'), $values);
      if (sizeof($parent_id) == 0) {
        if (!in_array($parent, $parent_cache)) {
          print "The parent $parent is not exist in table.\n";
          exit;
        }
      }
    }

    $parent_cache[] = $family;
    $family_cache[] = array(
       $parent, 
       $family, 
       $description
    );
  }
  fclose($fh);  

  return $family_cache;
}

/**
 * Load family_table into chado database
 */
function tripal_gene_family_family_table_loader($family_table) {

  // check the file exist (double check)
  if (!is_readable($family_table)) {
    watchdog('tripal_gene_family_loader',
      'Could not open the file :family_table. Check that file exists and that permissions are correct .',
      array(':family_table' => $family_table),
      WATCHDOG_ERROR);
    exit;
  }

  // check and load family_table to array 
  $family_cache = tripal_gene_family_family_table_check($family_table);
  $num_total = sizeof($family_cache);

  if ($num_total == 0) {
    watchdog('tripal_gene_family_loader',
      'No record can be found in classsify_table :family_table. Check the content of this file .',
      array(':family_table' => $family_table),
      WATCHDOG_ERROR);
    exit;
  }

  $num_insert = 0;
  $num_update = 0; 
  $nup_top_level = 0;

  // load the family_table to database
  $transaction = db_transaction();
  print "\nNOTE: Loading family_table and classify_table is performed using a database transaction. \n" .
     "If the load fails or is terminated prematurely then the entire set of \n" .
     "insertions/updates is rolled back and will not be found in the database\n\n";
  try {
    foreach ($family_cache as $record) {
      $parent = $record[0];
      $family = $record[1];
      $description = $record[2];

      // check and get parent id
      $parent_id = null;
      if (strcmp($parent, '0')) {
        $values = array(
          'name' => $parent,
          'cv_id' => array(
            'name' => 'tripal_gene_family',
          ),
        );
     
        $parent_result = chado_select_record('cvterm', array('cvterm_id'), $values);
        if (sizeof($parent_result) == 0) {
          watchdog('tripal_gene_family_loader',
            'The parent family :parent is not exist in table due to fail insert .',
            array(':parent' => $parent),
            WATCHDOG_ERROR);
            exit;
        }
        $parent_id = $parent_result[0]->cvterm_id;
      } else {
        $parent_id = 0; // top level family with 0 parent_name and parent_id
      }

      // report error when there is no parent id 

      // insert/update cvterm, if the term exist, it will be updated automatically
      $cvterm = tripal_insert_cvterm(array(
        'name' => $family,
        'definition' => $description,
        'cv_name' => 'tripal_gene_family',
        'db_name' => 'tripal_gene_family',
      ));
      $cvterm_id = $cvterm->cvterm_id;
      // print "kkk cvtermid:". $cvterm_id . "\n";

      // insert/update relationship
      // one family only have one parent family
      if ($parent_id > 0) {
        $values = array(
          'type_id' => array (
            'cv_id' => array (
              'name' => 'tripal_gene_family_relationship',
            ),
            'name' => 'member_of',
          ),
          'subject_id' => $cvterm_id,
          'object_id' => $parent_id,
        );

        $sql = "SELECT cvterm_relationship_id FROM chado.cvterm_relationship WHERE subject_id=:subject_id";
        $cvterm_relationship_id = db_query($sql, array(':subject_id'=>$cvterm_id))->fetchField();
        if ($cvterm_relationship_id) {
          $umatch = array('cvterm_relationship_id' => $cvterm_relationship_id);
          $result_update_rel = chado_update_record('cvterm_relationship', $umatch, $values);
          $num_update++;
        } 
        else {
          $result_insert_rel = chado_insert_record('cvterm_relationship', $values);
          $num_insert++;
        }
      } 
      else {
        $num_top_level++;
      }

      $num_family_process = $num_update + $num_insert + $num_top_level;

      // report 
      print  "Insert Update family : " . $num_family_process . 
         ", Insert relationship: " . $num_insert .
         ", Update relationship: " . $num_update .
         ", Top level family: " . $num_top_level .
         ", Total " . $num_total . ". Memory: " .
      number_format(memory_get_usage()) . " bytes.\r";
    }
  }
  catch (Exception $e) {
    print "\n";
    watchdog_exception('tripal_gene_family_loader', $e);
    $transaction->rollback();
    print "FAILED: Rolling back database changes...\n";
  }
  print "\nDone.\n";
  return;
}

/**
 * check classify table
 * @param classify_table
 * @return array of gene classification  
 */
function tripal_gene_family_classify_table_check($classify_table) {

  // check and load classify_table to array 
  $classify_cache = array(); // save the classify to table  
  $fh = fopen($classify_table, 'r');
  while($line = fgetcsv($fh, 0, "\t")) {
    if (sizeof($line) < 2) {
      print "The column number of classify_table is not correct.\n";
      exit;
    }
    $feature = $line[0];
    $family = $line[1];

    // check feature exist in unique name 
    $sql = "SELECT feature_id FROM chado.feature WHERE uniquename = :uniquename";
    $feature = trim($feature);
    $result_id = db_query($sql, array(':uniquename' => $feature))->fetchAll();
    if (sizeof($result_id) > 1) {
      print  "Duplicate feature: $feature.\n";
      exit;
    }
    if (sizeof($result_id) == 0 ) {
      print "Can not find feature: $feature.\n"; 
      exit;
    }

    $feature_id = $result_id[0]->feature_id;

    // check if family exist
    $values = array(
      'name' => $family,
      'cv_id' => array(
        'name' => 'tripal_gene_family',
      ),
    );

    $family_id = chado_select_record('cvterm', array('cvterm_id'), $values);
    if (!$family_id) {
      print "The family $family does not exist in chado database\n";
      exit;
    }
    $type_id = $family_id[0]->cvterm_id;

    $classify_cache[] = array(
      'feature_id' => $feature_id,
      'uniquename' => $feature,
      'type_id' => $type_id,
      'family' => $family
    );
  }
  fclose($fh);

  return $classify_cache;
}

/**
 * tripal job load gene classification to database
 */
function tripal_gene_family_classify_table_loader($classify_table) {

  // check classify_table 
  if (!is_readable($classify_table)) {
    watchdog('tripal_gene_family_loader',
      'Could not open the file :classify_table. Check that file exists and that permissions are correct .',
      array(':classify_table' => $classify_table),
      WATCHDOG_ERROR);
    exit;
  }

  $classify_cache = tripal_gene_family_classify_table_check($classify_table);
  $num_total = sizeof($classify_cache);

  if ($num_total == 0) {
    watchdog('tripal_gene_family_loader',
      'No record can be found in classsify_table :classify_table. Check the content of this file .',
      array(':classify_table' => $classify_table),
      WATCHDOG_ERROR);
    exit;
  }

  $num_feature = 0;
  $num_insert = 0;
  $num_update = 0;

  // load classify_table
  print "\nNOTE: Loading classify_table is performed using a database transaction. \n" .
     "If the load fails or is terminated prematurely then the entire set of \n" .
     "insertions/updates is rolled back and will not be found in the database\n\n";
  try {
    foreach ($classify_cache as $classify) {
      // check record 
      $sql = "SELECT featureprop_id FROM chado.featureprop WHERE feature_id =:feature_id AND type_id =:type_id";
      $arg = array(':feature_id' => $classify['feature_id'], ':type_id' => $classify['type_id']);
      $result = db_query($sql, $arg)->fetchAll(); 

      $values = array(
        'feature_id' => $classify['feature_id'], 
        'type_id' => $classify['type_id'],
        'rank' => 0,
      );

      // update
      if (sizeof($result) > 0) {
        $umatch = array(
          'featureprop_id' => $result[0]->featureprop_id,
        ); 
        $result_update = chado_update_record('featureprop', $umatch, $values);
        if (!$result_update) {
          watchdog('tripal_gene_family_loader',
            'Unable to update :feature to :family .',
            array(':feature' => $classify['uniquename'], ':family' => $classify['family']),
            WATCHDOG_ERROR);
          exit;
        }
        $num_update++;
      } 
      // insert 
      else {
        $result_insert = chado_insert_record('featureprop', $values);
        if (!$result_insert) {

          watchdog('tripal_gene_family_loader',
            'Unable to assign :feature to :family.',
            array(':feature' => $classify['uniquename'], ':family' => $classify['family']),
            WATCHDOG_ERROR);
          exit;
        } 
        $num_insert++;
      }
      $num_feature++;

      print  "Insert gene classification: " . $num_insert . ", Update gene classification: " . $num_update .
          ", Total " . $num_total . ". Memory: " .
          number_format(memory_get_usage()) . " bytes.\r";

    }
  }
  catch (Exception $e) {
    print "\n";
    watchdog_exception('tripal_gene_family_loader', $e);
    $transaction->rollback();
    print "FAILED: Rolling back database changes...\n";
  }
  print "\nDone.\n";
  return;
}


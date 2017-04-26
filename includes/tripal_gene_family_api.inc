<?php

/**
 * @return breadcrumb array for tripal_gene_family admin page
 */
function tripal_gene_family_breadcrumb() {
  $breadcrumb = array();
  $breadcrumb[] = l('Home', '<front>');
  $breadcrumb[] = l('Administration', 'admin');
  $breadcrumb[] = l('Tripal', 'admin/tripal');
  $breadcrumb[] = l('Extensions', 'admin/tripal/extension');
  $breadcrumb[] = l('Tripal Gene Family', 'admin/tripal/extension/tripal_gene_family');
  return $breadcrumb;
}

/**
 * @return array of families
 */
function tripal_gene_family_get_families() {
  $sql = "SELECT C.cvterm_id, C.name FROM chado.cvterm C 
    LEFT JOIN chado.cv CV ON C.cv_id = CV.cv_id
    WHERE CV.name = :cv_name";
  $result = db_query($sql, array(':cv_name'=> 'tripal_gene_family'))->fetchAll();
  $families = array();
  foreach ($result as $r) {
    $families[$r->cvterm_id] = $r->name;
  }
  return $families;
} 

/**
 * @return array of family structure
 */
function tripal_gene_family_get_structure() {

  $families = tripal_gene_family_get_families();
 
  $sql = "SELECT R.subject_id, R.object_id FROM chado.cvterm_relationship R 
    LEFT JOIN chado.cvterm CT ON R.type_id = CT.cvterm_id
    LEFT JOIN chado.cv CV ON CT.cv_id = CV.cv_id
    WHERE CV.name = :cv_name and CT.name = :ct_name";
  $args = array(':cv_name'=> 'tripal_gene_family_relationship', ':ct_name'=> 'member_of');
  $results = db_query($sql, $args)->fetchAll();

  // convert result to subject and object array
  $subjects = array();
  $objects = array();

  foreach ($results as $r) {
    $subjects[$r->subject_id] = $r->object_id;
    $objects[$r->object_id] = $r->subject_id;
  }

  // create family structure
  $family_stru = array();
  foreach ($families as $family_id => $family_name) {
    if (isset($subjects[$family_id])) {
      $obj_id = $subjects[$family_id];
      $obj_name = $families[$obj_id];

      if ( empty($family_stru[$obj_id]) ) {
        $family_stru[$obj_id] = new stdClass;
        $family_stru[$obj_id]->family_id = $obj_id;
        $family_stru[$obj_id]->family_name = $obj_name;
        $family_stru[$obj_id]->members = array();
      }
      $family_stru[$obj_id]->members[$family_id] = $families[$family_id];
    }
  }

  // packing family structure in one level, later change the code to do multi-level packing
  foreach ($family_stru as $family_obj) {
    foreach ($family_obj->members as $fid => $fname) {
      if ( isset($family_stru[$fid]) ) {
        $family_obj->members[$fid] = $family_stru[$fid];
        unset($family_stru[$fid]);
      }
    }
  }

  return $family_stru;
}


/**
 * @param organism id
 *
 * @param family id (cvterm id)
 *
 * @return gene list of specific families for specific organism
 */
function tripal_gene_family_get_list($organism_id, $family_id) {

  $where = array();
  $args = array();

  if ($organism_id > 0) {
    $where[] = "F.organism_id = :organism_id";
    $args[':organism_id'] = $organism_id;
  }

  if ($family_id > 0) {
    $where[] = "P.type_id = :type_id";
    $args[':type_id'] = $family_id;
  }

  $where_clause = '';
  if (sizeof($where) > 0) {
    $where_clause = 'WHERE ' . implode(' AND ', $where);
  }

  $sql = "SELECT O.common_name, O.organism_id, F.uniquename, F.feature_id, C.name as feature_type_name,
    T.name as family_name
    FROM chado.featureprop as P
    LEFT JOIN chado.feature F ON F.feature_id = P.feature_id  
    LEFT JOIN chado.organism O ON F.organism_id = O.organism_id
    LEFT JOIN chado.cvterm C ON F.type_id = C.cvterm_id
    LEFT JOIN chado.cvterm T ON P.type_id = T.cvterm_id
    $where_clause";
  
  $results = db_query($sql, $args)->fetchAll(); 

  return $results;
}

 

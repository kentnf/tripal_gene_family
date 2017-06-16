<?php

$results = tripal_gene_family_get_list($organism_id, $family_id);
//dpm($results);

// output html table 
if (sizeof($results) > 0) {

  $header_data = array( 'Organism', 'Feature', 'Family', 'Type');
  $rows_data = array();

  foreach ($results as $r) {
    $organism = l($r->common_name, '/organism/' . $r->organism_id );
    $feature = $r->uniquename;
    $nid = chado_get_nid_from_id('feature', $r->feature_id);
    if ($nid) {
      $feature = l($r->uniquename, '/node/' . $nid);
    } 

    $rows_data[] = array(
      $organism,
      $feature,
      $r->family_name,
      $r->feature_type_name,
    );
  }

  $header = array(
    'data' => $header_data,
  );

  $rows = array(
    'data' => $rows_data,
  );

  $variables = array(
    'header' => $header_data,
    'rows' => $rows_data,
    'attributes' => array('class' => 'table'),
  );

  $table_html = theme('table', $variables);


  print '<div class="row"> <div class="col-md-8 col-md-offset-2">';
  print $table_html;
  print '</div> </div>';
} 

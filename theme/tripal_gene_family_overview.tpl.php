<?php

// get family structure
$family_stru = tripal_gene_family_get_structure();

// output html 
$num_col = 6;

foreach ($family_stru as $family_top) {

  print '<h4>'.$family_top->family_name.'</h4>'; 
  
  $num = 0;
  $table_html = '<table class="table table-bordered"><tr>';
  foreach ($family_top->members as $fid => $fname) {
  
    if (gettype($fname) == 'object') {
      foreach ($fname->members as $sfid=> $sfname) {
        $num++;
        $fid_html = l($sfname, '/family/list/0/' . $sfid);
        $table_html .=  "<td>$fid_html</td>";
    
        if ($num % $num_col == 0)  {
          $table_html .=  "</tr><tr>";
        }
      }
    } 
    else {
      $num++;
      $fid_html = l($fname, '/family/list/0/' . $fid);
      $table_html .=  "<td>$fid_html</td>";

      if ($num % $num_col == 0)  {
        $table_html .=  "</tr><tr>";
      }
    }
  } 

  $fill_num = $num_col - ($num % $num_col);
  for($i=1; $i<=$fill_num; $i++) {
    $table_html .= "<td></td>";
  }
  $table_html .= "</tr></table>";
 
  print $table_html;
}


<?php
/**
 * @file
 * Functions used to install the module
 */

/**
 * Implements install_hook().
 *
 * Permforms actions when the module is first installed.
 *
 * @ingroup tripal_gene_family
 */
function tripal_gene_family_install() {

  $values = array (
    'name' => 'tripal_gene_family',
    'description' => 'db for tripal gene family module, used for adding cvterm for tripal sra module',
  );
  tripal_insert_db($values);

  tripal_insert_cv(
    'tripal_gene_family',
    'cvterm of gene falimies.'
  );

  tripal_insert_cv(
    'tripal_gene_family_relationship',
    'cvterm of relationship between gene families.'
  );

  $cvterm = tripal_insert_cvterm(array(
    'name' => 'member_of',
    'definition' => 'member of tripal gene family',
    'cv_name' => 'tripal_gene_family_relationship',
    'db_name' => 'tripal_gene_family',
  ));
}


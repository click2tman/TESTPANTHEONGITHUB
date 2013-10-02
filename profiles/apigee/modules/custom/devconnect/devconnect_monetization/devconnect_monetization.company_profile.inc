<?php

use Apigee\Util\CacheFactory;
use Apigee\Util\Log;
use Apigee\Mint\BankDetail;
use Apigee\Mint\Organization;
use Apigee\Mint\DataStructures\DeveloperTnc;
use Apigee\Mint\TermAndCondition;
use Apigee\Mint\DataStructures\Address;
use Apigee\ManagementAPI\Developer;
use Apigee\Mint\Types\Country;
use Apigee\Mint\Types\BillingType;
use Apigee\Mint\Types\DeveloperType;

function devconnet_monetization_company_profile_tabs($user, $active_tab) {
  drupal_set_title(t('Company Profile'));
  $me = module_exists('me') ? 'me' : $GLOBALS['user']->uid;
  $base_path = 'users/' . $me . '/monetization/company';
  $tabs = array(
    'attributes' => array(
      array(
        'name' => 'id',
        'value' => 'company_settings_tabs',
      ),
    ),
    'tabs' => array(
      $base_path . '/edit' => array(
        'text' => t('Contact Info'),
        'access callback' => 'user_access',
        'access args' => array('edit company profile'),
        'pane' => array ('class' => 'company-info'),
        'active' => 'edit' == $active_tab,
        'callback' => 'drupal_get_form',
        'callback args' => array('devconnect_monetization_company_details_form'),
      ),
      $base_path . '/bank-details' => array(
        'text' => t('Bank &amp; Finance'),
        'access callback' => 'user_access',
        'access args' => array('edit bank details'),
        'active' => 'bank-details' == $active_tab,
        'pane' => array ('class' => 'bank-info'),
        'callback' => 'drupal_get_form',
        'callback args' => array('devconnect_monetization_company_bank_details_form'),
      ),
      $base_path . '/tncs' => array(
        'text' => t('Terms &amp; Conditions'),
        'access callback' => 'user_access',
        'access args' => array('view terms and conditions'),
        'pane' => array ('class' => 'tnc'),
        'active' => 'tncs' == $active_tab,
        'callback' => 'devconnect_monetization_list_tncs',
      ),
      $base_path . '/user-roles' => array(
        'text' => t('Manage User Roles'),
        'access callback' => 'user_access',
        'access args' => array('edit developers roles'),
        'pane' => array ('class' => 'user-roles'),
        'active' => 'user-roles' == $active_tab,
        'callback' => 'devconnect_monetization_company_users_forms',
      ),
    )
  );
  if (!array_key_exists($base_path . '/' . $active_tab, $tabs['tabs'])) {
    drupal_not_found();
  }
  module_load_include('inc', 'devconnect_monetization', 'devconnect_monetization.tabs');
  $output = devconnect_monetization_theme_tabs($tabs);
  return $output;
}

function devconnect_monetization_company_users_forms () {
  // Manage Users tab
  $manage_users_form = drupal_get_form('devconnect_monetization_roles_form');
  $associate_new_user_form = drupal_get_form('devconnect_monetization_users_add_to_company_form');

  $variables = array(
    'manage_users_form'       => drupal_render($manage_users_form),
    'associate_new_user_form' => drupal_render($associate_new_user_form),
  );
  $output = theme('devconnect_monetization_company_users', $variables);
  return $output;
}

function devconnect_monetization_company_details_form () {
  $developer_id = _devconnect_monetization_get_developer_id(TRUE);
  $client = devconnect_default_api_client();
  $developer = new \Apigee\Mint\Developer($client);
  $developer->load($developer_id);

  $dev = new Developer($client);
  $dev->load($developer_id);

  $form['#attributes']['class'][] = 'form-horizontal';

  $form['company_details'] = array(
    '#type' => 'fieldset',
    '#title' => t('Company Details'),
    '#collapsible' => FALSE,
  );

  $form['company_details']['legal_company_name'] = array(
    '#title' => t('Legal Company Name'),
    '#type' => 'textfield',
    '#required' => TRUE,
    '#default_value' => $dev->getAttribute('MINT_DEVELOPER_LEGAL_NAME'),
  );

  $form['company_details']['company_reg_number'] = array(
    '#title' => t('Company Reg. Number'),
    '#type' => 'textfield',
    '#default_value' => $dev->getAttribute('MINT_REGISTRATION_ID'),
  );

  foreach($developer->getAddresses() as $address_item) {
    if ($address_item->isPrimary()) {
      $address = $address_item;
      break;
    }
  }

  if (!isset($address)) {
    $address = new Address(array('country' => 'US', 'isPrimary' => TRUE));
  }

  $form['billing_address'] = array(
    '#type' => 'fieldset',
    '#title' => t('Billing Address'),
    '#collapsible' => FALSE,
  );

  $form['address_id'] = array(
    '#type' => 'value',
    '#value' => $address->getId(),
  );
  $form['billing_address']['street_address_1'] = array(
    '#title' => t('Street Address 1'),
    '#type' => 'textfield',
    '#default_value' => $address->getAddress1(),
    '#required' => TRUE,
  );

  $form['billing_address']['street_address_2'] = array(
    '#title' => t('Street Address 2'),
    '#type' => 'textfield',
    '#default_value' => $address->getAddress2(),
  );

  $form['billing_address']['country'] = array(
    '#title' => t('Country'),
    '#type' => 'select',
    '#options' => Country::getList(),
    '#default_value' => $address->getCountry(),
    '#empty_option' => t('Select Country'),
  );

  $form['billing_address']['state_province'] = array(
    '#title' => t('State\Province'),
    '#type' => 'textfield',
    '#default_value' => $address->getState(),
  );

  $form['billing_address']['city'] = array(
    '#title' => t('City'),
    '#type' => 'textfield',
    '#default_value' => $address->getCity(),
    '#required' => TRUE,
  );

  $form['billing_address']['zip_code'] = array(
    '#title' => t('Zip/Postal Code'),
    '#type' => 'textfield',
    '#default_value' => $address->getZip(),
    '#required' => TRUE,
  );

  $form['_billing_type_fieldset'] = array(
    '#title' => t('Billing Type'),
    '#type' => 'fieldset',
    '#collapsible' => FALSE,
  );

  $form['_billing_type_fieldset']['_billing_type'] = array(
    '#markup' => ucfirst(strtolower($dev->getAttribute('MINT_BILLING_TYPE'))),
  );

  $form['contact_info'] = array(
    '#type' => 'fieldset',
    '#title' => t('Contact Info'),
    '#collapsible' => FALSE,
  );

  $form['contact_info']['contact_name'] = array(
    '#type' => 'item',
    '#title' => t('Contact Name'),
    '#markup' => t('@first_name @last_name', array('@first_name' => $dev->getFirstName(), '@last_name' => $dev->getLastName())),
  );

  $form['contact_info']['contact_tel_number'] = array(
    '#title' => t('Telephone Number'),
    '#type' => 'textfield',
    '#default_value' => $dev->getAttribute('MINT_DEVELOPER_PHONE'),
  );

  $form['contact_info']['contact_email'] = array(
    '#type' => 'item',
    '#title' => t('Email Address'),
    '#markup' => $developer->getEmail(),
  );

  $form['vat_and_tax_detail'] = array(
    '#title' => t('VAT &amp; Tax Details'),
    '#type' => 'fieldset',
    '#collapsible' => FALSE,
  );

  $form['vat_and_tax_detail']['registered_for_tax_vat'] = array(
    '#title' => t('Are you registered for Tax/VAT?'),
    '#type' => 'radios',
    '#options' => array(1 => t('Yes'), 0 => t('No')),
    '#default_value' => strlen(trim($dev->getAttribute('MINT_TAX_EXEMPT_AUTH_NO'))) > 0 ? 1 : 0,
  );

  $form['vat_and_tax_detail']['vat_tax_number'] = array(
    '#type' => 'textfield',
    '#title' => t('VAT/Tax Number'),
    '#default_value' => trim($dev->getAttribute('MINT_TAX_EXEMPT_AUTH_NO')),
  );

  $form['developer_type'] = array(
    '#type' => 'value',
    '#value' => $dev->getAttribute('MINT_DEVELOPER_TYPE'),
  );

  $form['billing_type'] = array(
    '#type' => 'value',
    '#value' => $dev->getAttribute('MINT_BILLING_TYPE'),
  );

  $form['company_id'] = array(
    '#type' => 'value',
    '#value' => $dev->getAttribute('MINT_COMPANY_ID'),
  );

  $form['approx_tax_rate'] = array(
    '#type' => 'value',
    '#value' => $dev->getAttribute('MINT_APPROX_TAX_RATE'),
  );

  // Note that KMS does not return boolean values but it string representation
  // That will break custom fields if they are checkboxes
  $form['is_broker'] = array(
    '#type' => 'value',
    '#value' => $dev->getAttribute('MINT_IS_BROKER') == 'true' ? 1 : 0,
  );

  $form['has_self_billing'] = array(
    '#type' => 'value',
    '#value' => $dev->getAttribute('MINT_HAS_SELF_BILLING'),
  );

  $form['billing_profile'] = array(
    '#type' => 'value',
    '#value' => $dev->getAttribute('MINT_BILLING_PROFILE'),
  );

  $form['supported_currency'] = array(
    '#type' => 'value',
    '#value' => $dev->getAttribute('MINT_SUPPORTED_CURRENCY'),
  );

  $form['developer_category'] = array(
    '#type' => 'value',
    '#value' => $dev->getAttribute('MINT_DEVELOPER_CATEGORY'),
  );

  $form['developer_roles'] = array(
    '#type' => 'value',
    '#value' => $dev->getAttribute('MINT_ROLES'),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save Changes'),
  );

  $form['cancel'] = array(
    '#markup' => l(t('Cancel'), 'users/' . $GLOBALS['user']->uid . '/monetization/company/edit'),
  );

  $form['#validate'][] = 'devconnect_monetization_company_details_form_validate';
  $form['#submit'][] = 'devconnect_monetization_company_details_form_submit';
  return $form;
}

function devconnect_monetization_company_details_form_validate(&$form, &$form_state) {
  if ($form_state['values']['registered_for_tax_vat'] == 1 && strlen(trim($form_state['values']['vat_tax_number'])) == 0) {
    form_set_error('vat_tax_number', t('If you are registered for Tax/VAT, then you must enter your VAT/Tax number'));
  }
}

function devconnect_monetization_company_details_form_submit(&$form, &$form_state) {
  try {
    $developer_id = _devconnect_monetization_get_developer_id(TRUE);
    if ($GLOBALS['user']->mail != $developer_id) {
      $uid = user_load_by_mail($developer_id)->uid;
    }
    else {
      $uid = $GLOBALS['user']->uid;
    }

    $account = entity_load_single('user', $uid);

    $account->field_mint_developer_legal_name[LANGUAGE_NONE][0]['value'] = $form_state['values']['legal_company_name'];
    $account->field_mint_registration_id[LANGUAGE_NONE][0]['value'] = $form_state['values']['company_reg_number'];
    if ($form_state['values']['registered_for_tax_vat'] == 1) {
      $account->field_mint_vat_tax_number[LANGUAGE_NONE][0]['value'] = $form_state['values']['vat_tax_number'];
    }
    $account->field_mint_developer_phone[LANGUAGE_NONE][0]['value'] = $form_state['values']['contact_tel_number'];
    $address = array(
      'address1' => $form_state['values']['street_address_1'],
      'address2' => $form_state['values']['street_address_2'],
      'city' => $form_state['values']['city'],
      'country' => $form_state['values']['country'],
      'isPrimary' => 'true',
      'state' => $form_state['values']['state_province'],
      'zip' => $form_state['values']['zip_code'],
    );
    if (isset($form_state['values']['address_id']) && !is_null($form_state['values']['address_id'])) {
      $address['id'] = $form_state['values']['address_id'];
    }
    $account->field_mint_developer_address[LANGUAGE_NONE][0]['value'] = json_encode($address, JSON_FORCE_OBJECT);
    $account->field_mint_developer_type[LANGUAGE_NONE][0]['value'] = $form_state['values']['developer_type'];
    $account->field_mint_billing_type[LANGUAGE_NONE][0]['value'] = $form_state['values']['billing_type'];
    $account->field_mint_company_id[LANGUAGE_NONE][0]['value'] = $form_state['values']['company_id'];
    $account->field_mint_approx_tax_rate[LANGUAGE_NONE][0]['value'] = $form_state['values']['approx_tax_rate'];
    $account->field_mint_is_broker[LANGUAGE_NONE][0]['value'] = $form_state['values']['is_broker'];
    $account->field_mint_has_self_billing[LANGUAGE_NONE][0]['value'] = $form_state['values']['has_self_billing'];
    $account->field_mint_billing_profile[LANGUAGE_NONE][0]['value'] = $form_state['values']['billing_profile'];
    $account->field_mint_supported_currency[LANGUAGE_NONE][0]['value'] = $form_state['values']['supported_currency'];
    $account->field_mint_developer_category[LANGUAGE_NONE][0]['value'] = $form_state['values']['developer_category'];
    $account->field_mint_developer_roles[LANGUAGE_NONE][0]['value'] = $form_state['values']['developer_roles'];

    entity_save('user', $account);
    drupal_set_message(t('Your Company Profile Info has been saved.'), 'status');
    module_invoke_all('devconnect_monetization_developer_upgraded_to_company', $account);
  }
  catch(\Exception $e) {
    drupal_set_message(t('The website encountered an unexpected error. Please try again later.'), 'error');
    Apigee\Util\Log::write('devconnect_monetization', Apigee\Util\Log::LOGLEVEL_CRITICAL, $e);
  }
}

function devconnect_monetization_company_bank_details_form() {

  $developer_id = _devconnect_monetization_get_developer_id(TRUE);
  $client = devconnect_monetization_default_api_client();

  $developer = new \Apigee\Mint\Developer($client);
  $developer->setEmail($developer_id);
  $bank_detail = $developer->getBankDetails();

  $organization = new Organization($client);
  $currencies = $organization->listSupportedCurrencies();

  $address = $bank_detail->getAddress();

  $form['#attributes']['class'][] = 'form-horizontal';

  $form['bank_details'] = array(
    '#type' => 'fieldset',
    '#title' => t('Bank Details'),
    '#collapsible' => FALSE,
  );

  $form['#bank_details'] = $bank_detail;

  $form['bank_details']['street_address_1'] = array(
    '#title' => t('Street Address 1'),
    '#type' => 'textfield',
    '#default_value' => isset($address) ? $address->getAddress1() : NULL,
  );

  $form['bank_details']['street_address_2'] = array(
    '#title' => t('Street Address 2'),
    '#type' => 'textfield',
    '#default_value' => isset($address) ? $address->getAddress2() : NULL,
  );

  $form['bank_details']['country'] = array(
    '#title' => t('Country'),
    '#type' => 'select',
    '#options' => Country::getList(),
    '#empty_option' => t('Select Country'),
    '#required' => TRUE,
    '#default_value' => isset($address) ? $address->getCountry() : NULL,
  );

  $form['bank_details']['state_province'] = array(
    '#title' => t('State\Province'),
    '#type' => 'textfield',
    '#default_value' => isset($address) ? $address->getState() : NULL,
  );

  $form['bank_details']['city'] = array(
    '#title' => t('City'),
    '#type' => 'textfield',
    '#default_value' => isset($address) ? $address->getCity() : NULL,
  );

  $form['bank_details']['zip_code'] = array(
    '#title' => t('Zip/Postal Code'),
    '#type' => 'textfield',
    '#default_value' => isset($address) ? $address->getZip() : NULL,
  );

  $form['account_info'] = array(
    '#title' => t('Account Info'),
    '#type' => 'fieldset',
    '#collapsible' => FALSE,
  );

  $form['account_info']['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Bank Name'),
    '#required' => TRUE,
    '#default_value' => $bank_detail->getName(),
  );

  $form['account_info']['account_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Account Name'),
    '#required' => TRUE,
    '#default_value' => $bank_detail->getAccountName(),
  );

  $form['account_info']['account_number'] = array(
    '#type' => 'textfield',
    '#title' => t('Account Number'),
    '#required' => TRUE,
    '#default_value' => $bank_detail->getAccountNumber(),
  );

  $currency_opts =array();
  foreach ($currencies as &$currency) {
    $currency_opts[$currency->id] = "{$currency->displayName} ({$currency->name})";
  }
  $form['account_info']['account_currency'] = array(
    '#type' => 'select',
    '#title' => t('Currency'),
    '#empty_option' => t('- Select currency -'),
    '#options' => $currency_opts,
    '#required' => TRUE,
    '#default_value' => $bank_detail->getCurrency(),
  );

  $form['account_info']['account_sort_code'] = array(
    '#type' => 'textfield',
    '#title' => t('Sort Code'),
    '#default_value' => $bank_detail->getSortCode(),
  );

  $form['account_info']['account_aban_routing_number'] = array(
    '#type' => 'textfield',
    '#title' => t('ABA/Routing Number'),
    '#default_value' => $bank_detail->getAban(),
  );

  $form['account_info']['account_bic_swiff_code'] = array(
    '#type' => 'textfield',
    '#title' => t('BIC/Swiff Code'),
    '#default_value' => $bank_detail->getBic(),
  );

  $form['account_info']['account_iban'] = array(
    '#type' => 'textfield',
    '#title' => t('IBAN'),
    '#default_value' => $bank_detail->getIbanNumber(),
  );

  $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Save Changes'),
  );

  $form['cancel'] = array(
      '#markup' => l(t('Cancel'), 'users/' . $GLOBALS['user']->uid . '/monetization/company/edit'),
  );

  $form['#validate'][] = 'devconnect_monetization_company_bank_details_form_validate';
  $form['#submit'][] = 'devconnect_monetization_company_bank_details_form_submit';
  return $form;
}

function devconnect_monetization_company_bank_details_form_validate(&$form, &$form_state) {
  $has_errors = FALSE;
  if ($form_state['values']['account_number'] != (string)((int)$form_state['values']['account_number'])) {
    form_set_error('account_number', t('Acount Number must be a numeric value with no decimals and no trailing spaces either.'));
  }
}

function devconnect_monetization_company_bank_details_form_submit(&$form, &$form_state) {
  $client = devconnect_monetization_default_api_client();
  $developer_id = _devconnect_monetization_get_developer_id(TRUE);

  try {
    $bank_detail = $form['#bank_details'];
    $address = $bank_detail->getAddress();
    $address = isset($address) ? $address : new Address();
    $address->setAddress1($form_state['values']['street_address_1']);
    $address->setAddress2($form_state['values']['street_address_2']);
    $address->setCountry($form_state['values']['country']);
    $address->setState($form_state['values']['state_province']);
    $address->setCity($form_state['values']['city']);
    $address->setZip($form_state['values']['zip_code']);

    $bank_detail->setAddress($address);
    $bank_detail->setName($form_state['values']['name']);
    $bank_detail->setAccountName($form_state['values']['account_name']);
    $bank_detail->setAccountNumber($form_state['values']['account_number']);
    $bank_detail->setCurrency($form_state['values']['account_currency']);
    $bank_detail->setSortCode($form_state['values']['account_sort_code']);
    $bank_detail->setAban($form_state['values']['account_aban_routing_number']);
    $bank_detail->setBic($form_state['values']['account_bic_swiff_code']);
    $bank_detail->setIbanNumber($form_state['values']['account_iban']);
    $bank_detail->save('save');
    drupal_set_message(t('Your Bank Details have been save.'), 'status');
  }
  catch (Exception $e) {
    Log::write('devconnect_monetization', Log::LOGLEVEL_ERROR, $e->getMessage());
    drupal_set_message(t('Your operation could not be completed. Please try again later.'), 'error');
  }
}

/* Monetization Roles */
function devconnect_monetization_roles_form() {

  $client = devconnect_monetization_default_api_client();
  $developer_id = _devconnect_monetization_get_developer_id(TRUE);

  $query = new EntityFieldQuery();
  $child_developers = $query
    ->entityCondition('entity_type', 'user')
    ->entityCondition('bundle', 'user')
    ->fieldCondition('field_mint_company_id', 'value', $developer_id, '=')
    ->execute();

  $child_ids = isset($child_developers['user']) && is_array($child_developers['user']) ? array_keys($child_developers['user']) : array();

  $developers = count($child_ids) > 0 ? entity_load('user', $child_ids) : array();

  $admin_uid = $developer_id == $GLOBALS['user']->mail ? $GLOBALS['user']->uid : user_load_by_mail($developer_id)->uid;
  $developers = entity_load('user', array($admin_uid)) + $developers;
  $form = array();
  $dev_list = array();
  foreach ($developers as $uid => $developer) {
    $dev_name = isset($developer->field_first_name[LANGUAGE_NONE][0]['value']) ? $developer->field_first_name[LANGUAGE_NONE][0]['value'] : '';
    $dev_name .= isset($developer->field_last_name[LANGUAGE_NONE][0]['value']) ? $developer->field_last_name[LANGUAGE_NONE][0]['value'] : '';
    $dev_list[$uid] = array(
      'name' => array(
        '#type' => 'item',
        '#markup' => $dev_name,
      ),
      "admin[$uid]" => array(
        '#type' => 'checkbox',
        '#default_value' => in_array(MONETIZATION_ADMIN_ROLE_NAME, $developer->roles),
      ),
      "finance[$uid]" => array(
        '#type' => 'checkbox',
        '#default_value' => in_array(MONETIZATION_FINANCE_ADMIN_ROLE_NAME, $developer->roles),
      ),
      "developer[$uid]" => array(
        '#type' => 'checkbox',
        '#default_value' => in_array(MONETIZATION_DEVELOPER_ROLE_NAME, $developer->roles),
      ),
      "action[$uid]" => array(
        '#markup' => l(t('Delete'), 'users/' . $GLOBALS['user']->uid . '/monetization/company/remove-developer/' . $uid, array('attributes' => array('class' => array('btn'))))
      ),
    );

    // Keep company admin and self user from editing its roles
    if ($developer->mail == $GLOBALS['user']->mail || $uid == $admin_uid) {
      if ($uid == $admin_uid) {
        $dev_list[$uid]['name'] += array(
          '#prefix' => '<strong>',
          '#suffix' => '</strong>',
        );
      }
      else {
        $dev_list[$uid]['name'] += array(
            '#prefix' => t('<div style="font-style:italic" title="This is you"><h4>'),
            '#suffix' => '</h4></div>',
        );
      }
      $dev_list[$uid]["admin[{$uid}]"]['#disabled'] = TRUE;
      $dev_list[$uid]["finance[{$uid}]"]['#disabled'] = TRUE;
      $dev_list[$uid]["developer[{$uid}]"]['#disabled'] = TRUE;
      $dev_list[$uid]["action[{$uid}]"]['#access'] = FALSE;
    }
  }

  $form['developers'] = $dev_list;

  $form['submit'][] = array(
    '#type' => 'submit',
    '#value' => 'Save',
  );
  $form['#theme'] = 'devconnect_monetization_roles_form';
  $form['#submit'][] = 'devconnect_monetization_roles_form_submit';
  return $form;
}

function devconnect_monetization_roles_form_submit($form, &$form_state) {
  try {

    $developer_id = _devconnect_monetization_get_developer_id(TRUE);

    $query = new EntityFieldQuery();
    $child_developers = $query
      ->entityCondition('entity_type', 'user')
      ->entityCondition('bundle', 'user')
      ->fieldCondition('field_mint_company_id', 'value', $developer_id, '=')
      ->execute();

    $child_ids = array_keys($child_developers['user']);

    $developers = !empty($child_ids) ? entity_load('user', $child_ids) : array();

    $pseudo_monetization_roles = array(
      'admin' => user_role_load_by_name(MONETIZATION_ADMIN_ROLE_NAME),
      'finance' => user_role_load_by_name(MONETIZATION_FINANCE_ADMIN_ROLE_NAME),
      'developer' => user_role_load_by_name(MONETIZATION_DEVELOPER_ROLE_NAME),
    );

    $monetization_roles = array();
    foreach ($pseudo_monetization_roles as $role) {
      $monetization_roles[$role->rid] = $role->name;
    }

    foreach ($developers as $uid => $developer) {
      $new_assigned_roles = array();
      foreach ($pseudo_monetization_roles as $pseudo_role_name => $role) {
        if (isset($form_state['input'][$pseudo_role_name][$uid]) && $form_state['input'][$pseudo_role_name][$uid] == 1) {
          $new_assigned_roles[$pseudo_monetization_roles[$pseudo_role_name]->rid] = $pseudo_monetization_roles[$pseudo_role_name]->name;
        }
      }

      $roles_the_user_has = array_intersect($developer->roles, $monetization_roles);
      $roles_user_will_keep = array_intersect($roles_the_user_has, $new_assigned_roles);

      $save_new_assigments = !($roles_user_will_keep == $roles_the_user_has) || !($roles_user_will_keep == $new_assigned_roles);

      if ($save_new_assigments) {
        foreach (array_keys($monetization_roles) as $rid) {
          unset($developer->roles[$rid]);
        }

        foreach ($new_assigned_roles as $rid => $name) {
          $developer->roles[$rid] = $name;
        }
        entity_save('user', $developer);
        module_invoke_all('devconnect_monetization_developer_assinged_mint_role', $developer);
      }
    }
    drupal_set_message(t('Operation completed successfully.'), 'status');
  }
  catch (Exception $e) {
    drupal_set_message(t('Your operation could not complete. Please contact your administrator if problem persists.'), 'error');
  }
}

function theme_devconnect_monetization_roles_form($variables) {
  $form = $variables['form'];
  $header = array(
    t('Username'),
    t('Admin'),
    t('Finance'),
    t('Developer'),
    t('Actions'),
  );

  $rows = array();

  foreach (element_children($form['developers']) as $uid) {
    $row = array();
    foreach (element_children($form['developers'][$uid]) as $element) {
      $row[] = drupal_render($form['developers'][$uid][$element]);
    }
    $rows[] = array('data' => $row);
  }

  $output = theme_table(array(
        'header' => $header,
        'rows' => $rows,
        'attributes' => array(),
        'caption' => '',
        'colgroups' => array(),
        'sticky' => TRUE,
        'empty' => t('Your company has no developers assigned.'),
      )) . drupal_render($form['submit']) . drupal_render_children($form);
  return $output;
}

function devconnect_monetization_users_add_to_company_form() {

  $form['user_email_address'] = array(
   '#type' => 'textfield',
   '#title' => t('Enter User Email Address'),
   '#required' => TRUE,
   //'#autocomplete_path' => 'users/' . $GLOBALS['user']->uid . '/monetization/developers',
 );

 $form['submit'] = array(
   '#type' => 'submit',
   '#value' => t('Add User'),
 );

  return $form;
}

function devconnect_monetization_users_add_to_company_form_validate(&$form, &$form_state) {
  $form_state['values']['user_email_address'] = trim($form_state['values']['user_email_address']);
  $admin_id = _devconnect_monetization_get_developer_id(TRUE);

  $admin = user_load_by_mail($admin_id);
  $admin = entity_load_single('user', $admin->uid);
  if (strlen($admin->field_mint_company_id[LANGUAGE_NONE][0]['value']) > 0) {
    form_set_error(NULL, t('You cannot add developers to your company since you are added to another company.'));
  }
  else {
    $form_state['admin_id'] = $admin_id;
  }

  if (trim($form_state['values']['user_email_address']) == $admin_id) {
    form_set_error('user_email_address', t('You cannot add your own user to your company.'));
  }

  $child_user = user_load_by_mail($form_state['values']['user_email_address']);
  if ($child_user == NULL) {
    form_set_error('user_email_address', t('User with email @mail does not exist.', array('@mail' => $form_state['values']['user_email_address'])));
  }
  else {
    $child_entity = entity_load_single('user', $child_user->uid);

    if (isset($child_entity->field_mint_company_id[LANGUAGE_NONE][0]['value'])
        && strlen($child_entity->field_mint_company_id[LANGUAGE_NONE][0]['value']) > 0
    ) {
      form_set_error('user_email_address', t('You can not add this developer since it is already assigned.'));
    }
    else {
      $form_state['child_entity'] = $child_entity;
    }
  }
}

function devconnect_monetization_users_add_to_company_form_submit($form, &$form_state) {
  try {
    $child_entity = $form_state['child_entity'];
    $child_entity->field_mint_company_id[LANGUAGE_NONE][0]['value'] = $form_state['admin_id'];
    entity_save('user', $child_entity);
    drupal_set_message(t('Developer @developer added to your company.', array('@developer' => $form_state['values']['user_email_address'])));
    module_invoke_all('devconnect_monetization_developer_added_to_company', $child_entity, $child_entity->field_mint_company_id[LANGUAGE_NONE][0]['value']);
  }
  catch (Exception $e) {
    drupal_set_message(t('There was an error trying to add @developer to your company', array('@developer' => $form_state['values']['user_email_address'])), 'error');
  }
}

function devconnect_monetization_users_autocomple($string) {
  $client = devconnect_monetization_default_api_client();
  $developer = new Developer($client);
  $developers = $developer->listDevelopers();
  $emails = array();
  foreach ($developers as $developer_item) {
    if (stripos($developer_item, $string) !== FALSE) {
      $emails[$developer_item] = $developer_item;
    }
  }
  return drupal_json_output($emails);
}

function devconnect_monetization_user_remove_from_company($uid) {
  try {
    $child_entity = entity_load_single('user', $uid);
    $company_id = $child_entity->field_mint_company_id[LANGUAGE_NONE][0]['value'];
    $child_entity->field_mint_company_id = NULL;
    entity_save('user', $child_entity);
    drupal_set_message(t('Developer @developer removed from your company.', array('@developer' => $child_entity->mail)));
    module_invoke_all('devconnect_monetization_developer_removed_from_company', $child_entity, $company_id);
    $me = module_exists('me') ? 'me' : $GLOBALS['user']->uid;
    drupal_goto("users/$me/monetization/company/user-roles");
  }
  catch (Exception $e) {
    drupal_set_message($e->getMessage(), 'error');
    drupal_set_message(t('There was an error trying to remove @developer from your company', array('@developer' => $form_state['values']['user_email_address'])), 'error');
  }
}

function devconnect_monetization_list_tncs() {

  $developer_id = _devconnect_monetization_get_developer_id(TRUE);
  $client = devconnect_monetization_default_api_client();
  $tnc = new TermAndCondition($client);
  $tncs = $tnc->getList(TRUE);

  $org = new Organization($client);
  $org->load();

  $org_timezone = new DateTimeZone($org->getTimezone());
  $local_timezone = new DateTimeZone(date_default_timezone_get());

  // Sort TnCs by start date DESC
  uasort($tncs, function($tnc1, $tnc2) {
    $date1 = date_create_from_format('Y-m-d', $tnc1->getStartDate());
    $date2 = date_create_from_format('Y-m-d', $tnc2->getStartDate());

    if ($date1 < $date2) return 1;
    if ($date1 > $date2) return -1;
    return 0;
  });

    $dev_tncs = $tnc->getAcceptedDevTermsAndConditions($developer_id);

    $tncs_forms = array();
    foreach ($tncs as $tnc) {
      $form = drupal_get_form('devconnect_monetization_accept_tnc_form', $tnc);
      $tncs_form = array(
          'tnc' => $tnc,
          'form' => drupal_render($form),
      );

      foreach ($dev_tncs as $dev_tnc) {
        if ($dev_tnc->getTnc()->getId() == $tnc->getId()) {
          $tncs_form['accepted'] = DateTime::createFromFormat('Y-m-d H:i:s', $dev_tnc->getAuditDate(), $org_timezone)->setTimezone($local_timezone)->format('Y-m-d H:i:s');
          break;
        }
      }
      $tncs_forms[] = $tncs_form;
    }

    /*********************************************************
     * If developers have not accepted TnC, they should only *
    * see the current and future TnC. Not old expired ones  *
    * (unless they had accepted them)                       *
    * Should only see the CURRENT (and next future) one.    *
    *********************************************************/

    $remove = FALSE;
    for ($i = 0, $len = count($tncs_forms); $i < $len; $i++) {
      if (!$remove && isset($tncs_forms[$i]['accepted'])) {
        $remove = TRUE;
        continue;
      }
      if ($remove && !isset($tncs_forms[$i]['accepted'])) {
        unset($tncs_forms[$i]);
      }
    }
    $variables = array(
      'tncs_forms' => $tncs_forms,
    );
    return theme('devconnect_monetization_company_tncs', $variables);
}

function devconnect_monetization_accept_tnc_form($form, &$form_state) {
  $tnc = $form_state['build_info']['args'][0];

  $form = array();
  if (user_access('accept terms and conditions')) {
    $form['tnc_id'] = array(
      '#type' => 'hidden',
      '#value' => $tnc->getId(),
    );
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Accept Terms & Conditions'),
    );
    $form['#submit'][] = 'devconnect_monetization_accept_tnc_form_submit';
  }
  return $form;
}

function devconnect_monetization_accept_tnc_form_submit($form, &$form_state) {
  try {
    $client = devconnect_monetization_default_api_client();
    $developer_id = _devconnect_monetization_get_developer_id(TRUE);
    $tnc_id = $form_state['input']['tnc_id'];

    $org = new Organization($client);
    $org->load();

    $org_timezone = new DateTimeZone($org->getTimezone());

    $dev_tnc = new DeveloperTnc();

    $audit_date = new DateTime('now');
    $dev_tnc->setAuditDate($audit_date->setTimezone($org_timezone)->format('Y-m-d H:i:s'));
    $dev_tnc->setAction('ACCEPTED');
    $tnc = new TermAndCondition($client);
    $tnc->acceptTermsAndConditions($dev_tnc, $developer_id, $tnc_id);
    CacheFactory::getCacheManager(NULL)->clear('developer_accepted_tncs:' . $developer_id);
    module_invoke_all('devconnect_monetization_developer_accepted_tncs', $tnc);
    drupal_set_message(t('You have accepted terms and conditions successfully.'));
  }
  catch (Exception $e) {
    drupal_set_message(t('Error while trying to accept TnCs. Please try again later.'));
    Apigee\Util\Log::write('devconnect_monetization', Apigee\Util\Log::LOGLEVEL_ERROR, 'Could not accept terms and conditions: ' . $e->getMessage());
  }
}
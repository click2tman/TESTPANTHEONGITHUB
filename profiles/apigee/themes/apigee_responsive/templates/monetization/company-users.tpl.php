<?php
/**
 * Variables:
 *   $manage_users_from Rendered form to remove user from a company or to grant them Mint Roles
 *   $associate_new_user_form Rendered form to add developers to a company
 */
?>
<div class="tab-pane user-roles" id="tab4">
  <div class="row">
    <div class="col-md-11">
      <h3>Manage Users</h3>
      <hr>
      <?php print $manage_users_form; ?>
    </div>
    <div class="col-md-11 offset2">
      <h3>Associate New User</h3>
      <hr>
      <?php print $associate_new_user_form; ?>
    </div>
  </div>
</div>
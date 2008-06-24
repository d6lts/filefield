// $Id$

/**
 *  Auto Attach standard client side file input validation
 */
Drupal.filefieldValidateAutoAttach = function() {
  $("input[@type='file']").change( function() {
    $('.filefield-js-error').remove();

    /**
     * Add client side validation for the input[@file] accept attribute.
     */
    if(this.accept.length > 1){
      v = new RegExp('\\.(' + (this.accept ? this.accept : '') + ')$', 'gi');
      if (!v.test(this.value)) {
        var error = 'The file ' + this.value + " is not supported.\n";
        error += "Only the following file types are supported: \n" + this.accept.replace(/\|/g, ' ');
        alert(error);
        // what do I prepend this to?
        // .prepend($('<div class="filefield-js-error>"' + error + '</div>'));
        this.value = '';
        return false;
      }
    }

    /**
     * Add filesize validation where possible.
     */
    /* @todo */
  });
}

// Global killswitch
if (Drupal.jsEnabled) {
  $(document).ready(Drupal.filefieldValidateAutoAttach);
}


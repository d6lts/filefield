// $Id$

/**
 * Auto-attach standard client side file input validation.
 */
Drupal.behaviors.filefieldValidateAutoAttach = function(context) {
  $("input[@type='file']", context).change( function() {
    $('.filefield-js-error').remove();

    /**
     * Add client side validation for the input[@file] accept attribute.
     */
    if(this.accept.length > 1){
      v = new RegExp('\\.(' + (this.accept ? this.accept : '') + ')$', 'gi');
      if (!v.test(this.value)) {
        var error = Drupal.t("The selected file %filename cannot not be uploaded. Only files with the following extensions are allowed: %extensions.",
          { '%filename' : this.value, '%extensions': this.accept.replace(/\|/g, ' ') }
        );
        // what do I prepend this to?
        $(this).before('<div class="messages error filefield-js-error">' + error + '</div>');
        this.files = '';
        return false;
      }
    }

    /**
     * Add filesize validation where possible.
     */
    /* @todo */
  });
}

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
        var error = Drupal.t("The file @filename is not supported. \nOnly the following file types are supported: \n@extensions",
          { '@filename' : this.value, '@extensions': this.accept.replace(/\|/g, ' ') }
        );
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

// $Id$

/**
 * Auto-attach standard client side file input validation.
 */
Drupal.behaviors.filefieldValidateAutoAttach = function(context) {
  $("input[type='file'][accept]", context).change( function() {
    // Remove any previous errors.
    $('.file-upload-js-error').remove();

    /**
     * Add client side validation for the input[type=file] accept attribute.
     */
    var accept = this.accept.replace(/,\s*/g, '|');
    if (accept.length > 1) {
      var v = new RegExp('\\.(' + accept + ')$', 'gi');
      if (!v.test(this.value)) {
        var error = Drupal.t("The selected file %filename cannot not be uploaded. Only files with the following extensions are allowed: %extensions.",
          { '%filename' : this.value, '%extensions' : accept.replace(/\|/g, ', ') }
        );
        // What do I prepend this to?
        $(this).before('<div class="messages error file-upload-js-error">' + error + '</div>');
        this.value = '';
        return false;
      }
    }

    /**
     * Add filesize validation where possible.
     */
    /* @todo */
  });
};

/**
 * Admin enhancement: only show the "Files listed by default" when needed.
 */
Drupal.behaviors.filefieldAdmin = function(context) {
  var $listField = $('div.filefield-list-field', context);
  if ($listField.size()) {
    $listField.find('input').change(function() {
      if (this.checked) {
        if (this.value == 0) {
          $('#edit-list-default-wrapper').css('display', 'none');
        }
        else {
          $('#edit-list-default-wrapper').css('display', 'block');
        }
      }
    }).change();
  }
};


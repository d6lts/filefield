filefield provides a image field type to cck.


if you are running drupal 4.7.3 you should update the .htaccess 
file in you files directory to make previews work...
better yet upgrade to 4.7.4.

  comment out RewriteEngine off
  add +FollowSymlinks to the Options stanza.



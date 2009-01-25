; $Id$

Link Checker
------------

To install, place the entire linkchecker folder into your modules directory.
Go to Administer -> Site building -> Modules and enable the Link checker module

Now go to Administer -> Site Configuration -> Link checker
and adjust the few parameters there if the defaults don't suit your needs.

The check results should appear after a while under this url:
http://example.com/admin/reports/linkchecker

If not, make sure the cron is configured and running properly on your Drupal
installation. The Link checker module also logs somewhat useful info about it's
activity under Administer -> Reports -> Recent log entries.

There is a bit more information at http://drupal.org/node/72840.

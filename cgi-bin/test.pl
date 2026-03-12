#!/usr/bin/perl
use strict;
use warnings;

# CGI headers
print "Content-Type: text/html\r\n\r\n";

# HTML output
print "<!DOCTYPE html>\n";
print "<html>\n";
print "<head>\n";
print "    <title>Perl CGI Test</title>\n";
print "    <style>\n";
print "        body { font-family: Arial; background: #f0f8ff; padding: 20px; }\n";
print "        .container { max-width: 800px; margin: 0 auto; background: white; padding: 30px; border-radius: 10px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); }\n";
print "        h1 { color: #8b4789; border-bottom: 3px solid #8b4789; padding-bottom: 10px; }\n";
print "        .info { background: #f9f9f9; padding: 15px; border-left: 4px solid #8b4789; margin: 15px 0; }\n";
print "        .success { color: #28a745; font-weight: bold; }\n";
print "        code { background: #f4f4f4; padding: 2px 6px; border-radius: 3px; }\n";
print "    </style>\n";
print "</head>\n";
print "<body>\n";
print "    <div class='container'>\n";
print "        <h1> Perl CGI Script</h1>\n";
print "        <p class='success'> Perl CGI is working!</p>\n";
print "        <div class='info'>\n";
print "            <h3>System Information:</h3>\n";
print "            <p><strong>Perl Version:</strong> $^V</p>\n";
print "            <p><strong>OS:</strong> $^O</p>\n";
print "            <p><strong>Time:</strong> " . localtime() . "</p>\n";
print "        </div>\n";
print "        <div class='info'>\n";
print "            <h3>CGI Environment Variables:</h3>\n";

# Print environment variables
foreach my $key (sort keys %ENV) {
    print "            <p><code>$key</code> = $ENV{$key}</p>\n";
}

print "        </div>\n";
print "        <p><a href='/'>← Back to Home</a></p>\n";
print "    </div>\n";
print "</body>\n";
print "</html>\n";

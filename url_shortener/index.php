<?php
require 'Slim/Slim.php';
\Slim\Slim::registerAutoloader();

$app = new \Slim\Slim(array(
   'cookies.encrypt' => true
));

// --- GET for the landing page/form ---

$app->get( '/', function ()  use ($app) {
    // get the root uri from the request so that the form works
    // even if this script isn't on the root public_html
    // directory
    $req = $app->request;
    $rootUri = $req->getRootUri();

     $template = <<<EOT
<!DOCTYPE html><html><head><meta charset="utf-8"/>
<title>Shorty</title>
<style>*{margin:15px auto;}body{text-align:center;}
</style>
</head><body>
            <h1>Shorten url:</h1>
             <form action="$rootUri/shorten" method="post">
                <input type="text" name="link"><br>
                <input type="submit" value="Submit">
            </form>
</body></html>
EOT;
     echo $template;
});

// --- POST handling saving of urls and producing their id  ---

$app->post('/shorten', function () use ($app)  {
    // get the posted url
    $paramUri = $app->request()->params('link');
    // sanitize it
    $paramUri = strip_tags(filter_var(filter_var(trim($paramUri) , FILTER_SANITIZE_SPECIAL_CHARS, FILTER_FLAG_STRIP_LOW), FILTER_SANITIZE_MAGIC_QUOTES));
    // check if it indeed is a URL
    if(filter_var($paramUri, FILTER_VALIDATE_URL)) {
        // if it is then hash it, store it in-memory and print the shortened link
        $compressed = substr(hash('sha256', $paramUri), 0, 8);
        apc_store($compressed . "", $paramUri);
        $urlScheme = $app->request()->getResourceUri();
        $actual_url = 'http' . (isset($_SERVER['HTTPS']) ? 's' : '') . "://$_SERVER[HTTP_HOST]/";

        echo $actual_url . $compressed;
    } else {
        // otherwise print error
        echo "Error: Invalid URL!";
    }

});

// ------------ GET handling id's of shortened urls -----------

$app->get('/:someuri', function ($someuri) use ($app) {
    // get given id and sanitize it
    $sUri = filter_var(filter_var(trim($someuri) , FILTER_SANITIZE_SPECIAL_CHARS, FILTER_FLAG_STRIP_LOW), FILTER_SANITIZE_MAGIC_QUOTES);
    // check if it is only alphanumeric characters as it should be
    if (!preg_match("/^[a-zA-Z0-9 ]+$/", $sUri)) {
        echo "Error: Invalid input! Use only alphanumeric characters.";
    } else {
        // if it is then fetch the shortened url from memory
        $success = true;
        $oldUrl = apc_fetch($sUri, $success);
        // if it exists in memory redirect to it
        if ($success) {
            $app->redirect( $oldUrl, 301);
        // otherwise send a 404 page by passing any other handling of the request
        } else {
            $app->pass();
        }
    }
});

$app->run();

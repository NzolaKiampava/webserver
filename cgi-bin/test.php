<?php
header("Content-Type: text/html");
?>
<!DOCTYPE html>
<html>
<head>
    <title>PHP CGI Test</title>
    <style>
        body { font-family: Arial; background: #f0f0ff; padding: 20px; }
        .container { max-width: 800px; margin: 0 auto; background: white; padding: 30px; border-radius: 10px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); }
        h1 { color: #4f5d95; border-bottom: 3px solid #4f5d95; padding-bottom: 10px; }
        .info { background: #f9f9f9; padding: 15px; border-left: 4px solid #4f5d95; margin: 15px 0; }
        .success { color: #28a745; font-weight: bold; }
        code { background: #f4f4f4; padding: 2px 6px; border-radius: 3px; }
        table { width: 100%; border-collapse: collapse; margin-top: 10px; }
        th, td { padding: 8px; text-align: left; border-bottom: 1px solid #ddd; }
        th { background: #4f5d95; color: white; }
    </style>
</head>
<body>
    <div class='container'>
        <h1> PHP CGI Script</h1>
        <p class='success'> PHP CGI is working!</p>
        
        <div class='info'>
            <h3>System Information:</h3>
            <p><strong>PHP Version:</strong> <?php echo phpversion(); ?></p>
            <p><strong>Server Software:</strong> <?php echo $_SERVER['SERVER_SOFTWARE'] ?? 'N/A'; ?></p>
            <p><strong>Server Protocol:</strong> <?php echo $_SERVER['SERVER_PROTOCOL'] ?? 'N/A'; ?></p>
            <p><strong>Current Time:</strong> <?php echo date('Y-m-d H:i:s'); ?></p>
        </div>
        
        <div class='info'>
            <h3>Request Information:</h3>
            <p><strong>Method:</strong> <?php echo $_SERVER['REQUEST_METHOD'] ?? 'N/A'; ?></p>
            <p><strong>URI:</strong> <?php echo $_SERVER['REQUEST_URI'] ?? 'N/A'; ?></p>
            <p><strong>Query String:</strong> <?php echo $_SERVER['QUERY_STRING'] ?? 'N/A'; ?></p>
            <p><strong>Remote Address:</strong> <?php echo $_SERVER['REMOTE_ADDR'] ?? 'N/A'; ?></p>
        </div>
        
        <div class='info'>
            <h3>CGI Environment Variables:</h3>
            <table>
                <tr>
                    <th>Variable</th>
                    <th>Value</th>
                </tr>
                <?php
                foreach ($_SERVER as $key => $value) {
                    if (is_string($value)) {
                        echo "<tr><td><code>$key</code></td><td>" . htmlspecialchars($value) . "</td></tr>";
                    }
                }
                ?>
            </table>
        </div>
        
        <p><a href='/'>← Back to Home</a></p>
    </div>
</body>
</html>

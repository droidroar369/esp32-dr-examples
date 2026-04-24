#include <WiFi.h>
#include <esp_http_server.h>
#include "credentials.h" //File with ssid and password WiFi credentials

#define RGB_BUILTIN 48 //RGB led

const char* html PROGMEM= R"rawliteral(
<html>
<head>
<meta charset="UTF-8">
<title>Color chooser</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
body {
  background-color: #666;
  display: flex;
  flex-direction: column;
  justify-content: center;
  align-items: center;
  gap: 8px;
}

p {
  color: white;
  font: normal normal 24px calligra, verdana;
}

#color01 {
  width: 80px;
  height: 40px;
  border: 1px solid #000;
  
  &::-webkit-color-swatch {
    border-radius: 25%;
    margin: -2px;
  }
  cursor: pointer;
}

#white-btn {
  width: 32px;
  height:32px;
  border: solid 2px black;
  cursor: pointer;
}

#black-btn {
  width: 32px;
  height:32px;
  background-color: black;
  background-image: url('data:image/svg+xml;utf8,<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 100 100"><path d="M 36.3 12.4 A 40 40 0 1 0 63.7 12.4 M 50 50 L 50 5" stroke="white" stroke-width="3"/></svg>');
  cursor: pointer;
}

.panel01 {
  display: flex;
  justify-content: center;
  align-items: center;
  gap: 15px;
  margin-top: -10px;
}

.panel02 {
  display: flex;
  flex-direction: column;
  justify-content: center;
  gap: 5px;
}
</style>
</head>

<body>

<p>Escoge un color</p>
<div class="panel01">
<input type="color" id="color01" onchange="send_color(this.value);">
<div class="panel02">
  <input type="button" id="white-btn" onclick="send_color('#ffffff');">
  <input type="button" id="black-btn" onclick="send_color('#000000');">
</div>
</div>

<script type="text/javascript">
//Function that sends the color through AJAX
function send_color(color) {
  var xhr= new XMLHttpRequest();
  color= color.substring(1); //Color without the initial "#"
  console.log(color);
  
  
  xhr.open("POST", "/get_color", true);
  xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded; charset=UTF-8');
  
  xhr.onreadystatechange = function() {
    if(xhr.readyState===4 && xhr.status===200) {
      console.log(xhr.responseText);
    }
  };
  
  xhr.send("color="+ encodeURIComponent(color));
}
</script>
</body>
</html>
)rawliteral";

httpd_handle_t server = NULL; //Server object
uint8_t r=255,g=255,b=255;


//Handle root / request
static esp_err_t handle_root(httpd_req_t* req) {
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, html, strlen(html));
}


//Handle /get_color request
static esp_err_t handle_get_color(httpd_req_t* req) {
  char* buf= NULL;
  size_t buf_len= req->content_len; //Length of data received
  
  if(buf_len>0) {
    buf= (char*) malloc(buf_len+1);
    
    if(!buf) {
      httpd_resp_send_500(req);
      return ESP_FAIL;
    }
    
    int r_len= httpd_req_recv(req, buf, buf_len); //Get request data
    if(r_len<0) {
      free(buf);
      httpd_resp_send_500(req);
      return ESP_FAIL;
    }
    buf[r_len]='\0'; //End of string
    
    uint32_t colors;
    sscanf(buf, "color=%x", &colors); //Extract colors
    r= (uint8_t)(colors >> 16); g= (uint8_t)((colors >> 8) & 0xFF); b= (uint8_t)(colors & 0xFF);
    
    Serial.printf("Color: #%x%x%x\n", r,g,b);
    rgbLedWrite(RGB_BUILTIN, r,g,b); //Change led color

    free(buf);

    httpd_resp_set_type(req, "text/plain");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, "Ok", 2);
  }
  else return ESP_FAIL;
}


// Function to config and start the server
void start_server() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.max_uri_handlers = 4;

  //URI handlers
  httpd_uri_t root_uri = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = handle_root,
    .user_ctx = NULL
  };
  
  httpd_uri_t get_color_uri = {
    .uri = "/get_color",
    .method = HTTP_POST,
    .handler = handle_get_color,
    .user_ctx = NULL
  };

  //Start server
  if(httpd_start(&server, &config)==ESP_OK) {
    //Register URI's
    httpd_register_uri_handler(server, &root_uri);
    httpd_register_uri_handler(server, &get_color_uri);
  } else {
    Serial.println("Error: server couldn't initialize");
  }
}


void setup() {
  // RGB led
  pinMode(RGB_BUILTIN, OUTPUT);
  
  //Begin serial
  Serial.begin(115200);
  delay(1000);
  
  //Create AP
  Serial.println("Creating Access Point");
  WiFi.softAP(ssid, password);
  Serial.println("Access point created. Connect to the IP "+ WiFi.softAPIP().toString());
  
  start_server();
}


void loop() {
  // All is managed by the server
}

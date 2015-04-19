from cStringIO import StringIO

def handler(request):
  html = StringIO()

  if "HttpStatusCode" in request.headers:
    http_status_code = int(request.headers["HttpStatusCode"])
  else:
    http_status_code = -2

  html.write("Request for http status code: " + str(http_status_code))

  if http_status_code >= 400:
    request.send_error(http_status_code)
  else:
    request.send_response(http_status_code)
    request.send_header("Content-type", "application/json")
    request.send_header("Content-Length", html.tell())
    request.send_header("Cache-Control", "no-cache")
    request.end_headers()

    request.wfile.write(html.getvalue())

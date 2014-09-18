def handler(request):
  request.send_response(302)
  request.send_header("Location", "empty.html")
  request.end_headers()

require 'socket'

port = 1222
s = TCPSocket.new('localhost', port)

led_info = gets.chomp!

s.puts('c#' + led_info)

info = nil
while not info do
  info = s.gets
end
while info do
  puts("LED #{info}")
  info = s.gets
end

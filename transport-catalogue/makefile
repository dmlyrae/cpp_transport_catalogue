CC = clang++
CFLAGS = -std=c++17 -fsanitize=undefined

SRCS = main.cpp request_handler.cpp svg.cpp json.cpp json_builder.cpp domain.cpp domain_render.cpp domain_transport.cpp domain_requests.cpp domain_responses.cpp map_renderer.cpp geo.cpp transport_catalogue.cpp

OBJS = $(SRCS:.cpp=.o)
EXEC = transport_catalogue

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(EXEC)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(EXEC)
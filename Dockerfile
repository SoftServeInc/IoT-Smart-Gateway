FROM alpine:3.12
LABEL description="IoT-Smart-Gateway IOx application"

# Install dependencies
RUN apk upgrade && apk add --no-cache boost-filesystem boost-regex boost-thread gnutls-dev

# Add working files
RUN mkdir /workspace
COPY ./cmake-build-debug/IoT-Smart-Gateway /workspace/IoT-Smart-Gateway
RUN chmod +x /workspace/IoT-Smart-Gateway
COPY ./web /workspace/web
RUN chmod 777 /workspace/web

# Start IOx app
WORKDIR /workspace
#CMD ["/workspace/IoT-Smart-Gateway"]
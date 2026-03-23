FROM golang:1.24-alpine AS builder

WORKDIR /app

COPY go.mod go.sum ./
RUN go mod download

COPY . .
RUN CGO_ENABLED=0 GOOS=linux go build -o /bin/controle_server .

FROM alpine:3.21

WORKDIR /app
COPY --from=builder /bin/controle_server /app/controle_server

EXPOSE 3000
CMD ["/app/controle_server"]

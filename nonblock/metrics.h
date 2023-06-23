#ifndef METRICS_H
#define METRICS_H

typedef struct metrics {
  int concurrent_connections;
  int historical_connections;
  int bytes_sent;
} metrics;

#endif
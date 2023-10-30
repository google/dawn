package utils

import (
	"context"
	"fmt"
	"os"
	"os/signal"
	"syscall"
)

// CancelOnInterruptContext returns a context that's cancelled if the process receives a SIGINT or
// SIGTERM interrupt.
func CancelOnInterruptContext(ctx context.Context) context.Context {
	ctx, cancel := context.WithCancel(context.Background())
	sigs := make(chan os.Signal, 1)
	signal.Notify(sigs, syscall.SIGINT, syscall.SIGTERM)
	go func() {
		sig := <-sigs
		fmt.Printf("Signal received: %v\n", sig)
		cancel()
	}()
	return ctx
}

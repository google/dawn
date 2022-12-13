// Copyright 2022 The Dawn Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

package cov

import (
	"bytes"
	"context"
	"fmt"
	"io"
	"net/http"
	"os"
	"path/filepath"
	"strings"

	"dawn.googlesource.com/dawn/tools/src/fileutils"
)

// StartServer starts a localhost http server to display the coverage data.
// Calls started() when the server is started, and then blocks until the context is cancelled.
func StartServer(ctx context.Context, port int, covData []byte, started func() error) error {
	ctx, stop := context.WithCancel(ctx)

	url := fmt.Sprintf("http://localhost:%v/index.html", port)
	handler := http.NewServeMux()
	handler.HandleFunc("/index.html", func(w http.ResponseWriter, r *http.Request) {
		f, err := os.Open(filepath.Join(fileutils.DawnRoot(), "tools/src/cov/view-coverage.html"))
		if err != nil {
			fmt.Fprint(w, "file not found")
			w.WriteHeader(http.StatusNotFound)
			return
		}
		defer f.Close()
		io.Copy(w, f)
	})
	handler.HandleFunc("/coverage.dat", func(w http.ResponseWriter, r *http.Request) {
		io.Copy(w, bytes.NewReader(covData))
	})
	handler.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
		rel := r.URL.Path
		if r.URL.Path == "" {
			http.Redirect(w, r, url, http.StatusSeeOther)
			return
		}
		if strings.Contains(rel, "..") {
			w.WriteHeader(http.StatusBadRequest)
			fmt.Fprint(w, "file path must not contain '..'")
			return
		}
		f, err := os.Open(filepath.Join(fileutils.DawnRoot(), r.URL.Path))
		if err != nil {
			w.WriteHeader(http.StatusNotFound)
			fmt.Fprintf(w, "file '%v' not found", r.URL.Path)
			return
		}
		defer f.Close()
		io.Copy(w, f)
	})
	handler.HandleFunc("/viewer.closed", func(w http.ResponseWriter, r *http.Request) {
		stop()
	})

	server := &http.Server{Addr: fmt.Sprint(":", port), Handler: handler}
	go server.ListenAndServe()

	if err := started(); err != nil {
		return err
	}

	<-ctx.Done()
	err := server.Shutdown(ctx)
	switch err {
	case nil, context.Canceled:
		return nil
	default:
		return err
	}
}

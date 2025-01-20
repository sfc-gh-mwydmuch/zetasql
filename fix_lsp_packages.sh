#importpath = "go.lsp.dev/jsonrpc2",
#importpath = "github.com/go-language-server/jsonrpc2",

#importpath = "go.lsp.dev/protocol",
#importpath = "github.com/go-language-server/protocol",

#importpath = "go.lsp.dev/uri",
#importpath = "github.com/go-language-server/uri",

#importpath = "go.lsp.dev/pkg",
#importpath = "github.com/go-language-server/pkg",

find . -type f -name "*.bzl" -exec sed -i '' 's/go.lsp.dev/github.com\/go-language-server/g' {} +
bazel --output_user_root ./.bazel_cache fetch -c opt //zetasql/tools/execute_query:execute_query
find . -type f -name "*.bzl" -exec sed -i '' 's/go.lsp.dev/github.com\/go-language-server/g' {} +
find . -type f -name "*.go" -exec sed -i '' 's/go.lsp.dev/github.com\/go-language-server/g' {} +
find . -type f -name "*.mod" -exec sed -i '' 's/go.lsp.dev/github.com\/go-language-server/g' {} +
find . -type f -name "*.sum" -exec sed -i '' 's/go.lsp.dev/github.com\/go-language-server/g' {} +
find . -type f -name "*.yml" -exec sed -i '' 's/go.lsp.dev/github.com\/go-language-server/g' {} +
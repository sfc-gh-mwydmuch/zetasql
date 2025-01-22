# Broken import paths in the go-language-server repository
#importpath = "go.lsp.dev/jsonrpc2",
# needs to be renamed to
#importpath = "github.com/go-language-server/jsonrpc2",

#importpath = "go.lsp.dev/protocol",
# needs to be renamed to
#importpath = "github.com/go-language-server/protocol",

#importpath = "go.lsp.dev/uri",
# needs to be renamed to
#importpath = "github.com/go-language-server/uri",

#importpath = "go.lsp.dev/pkg",
# needs to be renamed to
#importpath = "github.com/go-language-server/pkg",

BAZEL_USER_ROOT=$1
find . -type f -name "*.bzl" -exec sed -i 's/go.lsp.dev/github.com\/go-language-server/g' {} +
find . -type f -name "*.go" -exec sed -i 's/go.lsp.dev/github.com\/go-language-server/g' {} +
find . -type f -name "*.mod" -exec sed -i 's/go.lsp.dev/github.com\/go-language-server/g' {} +
find . -type f -name "*.sum" -exec sed -i 's/go.lsp.dev/github.com\/go-language-server/g' {} +
find . -type f -name "*.yml" -exec sed -i 's/go.lsp.dev/github.com\/go-language-server/g' {} +
FUTURE_DATE=$(date +%s -d "+10 years")
find ${BAZEL_USER_ROOT}/install -type f -exec touch --date=@${FUTURE_DATE} {} + # To prevent bazel from raporting the files as corrupted
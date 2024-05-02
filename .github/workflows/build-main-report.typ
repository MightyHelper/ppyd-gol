name: Build report
run-name: ${{ github.actor }} is building a report
on: [push]
jobs:
  check-bats-version:
    runs-on: self-hosted
    steps:
      - name: Checkout
        uses: actions/checkout@v4
      - name: Github Action for Typst
        uses: lvignoli/typst-action@v0.1
        with:
          source_file: reports/main.typ


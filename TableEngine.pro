TEMPLATE = subdirs
SUBDIRS += src main tests
main.depends = src
test.depends = src

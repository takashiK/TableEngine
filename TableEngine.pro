TEMPLATE = subdirs
SUBDIRS += src main tests viewer_tests
main.depends = src
test.depends = src
viewer_tests.depends = src

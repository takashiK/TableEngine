TEMPLATE = subdirs
SUBDIRS += src main tests viewer_document viewer_picture viewer_tests
src.depends = viewer_document viewer_picture
main.depends = src
test.depends = src
viewer_tests.depends = viewer_document viewer_picture

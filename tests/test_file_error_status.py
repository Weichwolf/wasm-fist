import test_file_loader_cf as carry


class FileErrorStatusTest(carry.FileLoaderCarryTest):
    """Both real DOS failures store the original complete WORD status."""

    OBSERVE_AFTER_STORES = True

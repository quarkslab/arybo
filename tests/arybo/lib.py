from arybo.lib import MBA
from arybo.lib import MBATester

import unittest

class Lib(unittest.TestCase):
    def test_lib(self):
        mba = MBA(4)
        mba_tester = MBATester(mba)
        ret = mba_tester.test_all()
        self.assertTrue(ret)

if __name__ == "__main__":
    unittest.main()

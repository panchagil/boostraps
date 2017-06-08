import numpy as np
import mymodule
a = np.array([[1, 2], [3, 4]], dtype=np.float64)
b = np.array([[1, 0], [0, 1]], dtype=np.float64)
c = mymodule.dummyM(a, b)
np.testing.assert_array_equal(c, a + b)

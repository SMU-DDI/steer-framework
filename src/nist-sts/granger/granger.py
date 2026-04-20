import sys
try:
    import statsmodels.api as sm
    from typing import Tuple
    from scipy import stats
    import numpy as np
    import argparse
except ImportError as e:
    print("Error: Missing library: " + str(e))
    sys.exit(1)

def import_bitstream(file_loc: str) -> np.array:
    """
    Conditions a raw .bit file into a format suitable for numpy operation. We expect files to be bit values in 
    byte notation, ie the file should only be bytes of 0x00 or 0x01
    :param file_loc: A string pointing to the file location.
    :return: A numpy array of "1s" and "0s"
    """

    return np.int64(np.fromfile(file_loc, dtype=np.uint8))

def format_bitstream(bitstream: list, n: int, num_offset_windows: int) -> Tuple[np.array, np.array, np.array]:
    """
    Formats the bitstream into two sequences, restricted and unrestricted. In the terms of
    the traditional Granger test, sequence[0] represents region "X" and sequence[1] represents region "Y".
    The restricted model consists of sequence[1], with the next proceeding bit in the sequence as the target
    bit. The unrestricted model consists of sequence[0] plus and the restricted model, including the
    target bit.

    :param bitstream: The input bitstream to be evaluated
    :param n: The number of bits per window. This is how many bits are used to predict the next bit for a
    given window.
    :param offset: The separation in number of bits between sequence[0] and sequence[1]

    :return: Tuple of numpy arrays, containing the restricted, unrestricted, and target bit lists
    """

    offset = num_offset_windows * n
    sequences = [[], []]
    target_bits = []

    for i in range(offset, len(bitstream) - n):
        sequences[1].append(bitstream[i:i + n])
        target_bits.append(bitstream[i + n])

    for i in range(0, len(target_bits)):
        sequences[0].append(bitstream[i:i + n])
    sequences[0] = np.array(sequences[0])

    restricted_set = np.array(sequences[1])
    unrestricted_set = np.concatenate((sequences[0], sequences[1]), axis=1)
    target_bits = np.array(target_bits)

    return restricted_set, unrestricted_set, target_bits


def granger_test(restricted_set: np.array, unrestricted_set: np.array,
                 target: np.array) -> Tuple[float, float]:
    """
    Performs the granger test on the formatted data
    :param restricted_set: Region Y in the traditional Granger test, or "recent" bits
    :param unrestricted_set: Region X in the traditional Granger test, or "past" bits
    :param target:
    :return: Tuple of the test log-likelihood ratio and the survival function
    """

    restricted_set = sm.add_constant(restricted_set)
    restricted_result = sm.Logit(target, restricted_set).fit(disp=False)

    unrestricted_set = sm.add_constant(unrestricted_set)
    unrestricted_result = sm.Logit(target, unrestricted_set).fit(disp=False)

    diff = restricted_result.llf - unrestricted_result.llf
    lr = -2 * (diff)
    sf = stats.chi2.sf(lr, unrestricted_set.shape[1] - restricted_set.shape[1])

    return diff, lr, sf


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Granger")
    parser.add_argument("-l", "--data_location", type=str, help="The location of the random number data stream")
    parser.add_argument("-n", "--window_size", type=int, help="The number of bits per window", default=8)
    parser.add_argument("-o", "--offset", type=int, help="Number of offset windows between the restricted window and target bit", default=6)
    args = parser.parse_args()

    nist_data_bits = import_bitstream(args.data_location)
    
    rs, us, t = format_bitstream(nist_data_bits, args.window_size, args.offset)
    diff, lr, pval = granger_test(rs, us, t)
    ones = (nist_data_bits == 0).sum()
    zeroes = (nist_data_bits == 1).sum()
    print("success", diff, lr, pval, ones, zeroes, end="")



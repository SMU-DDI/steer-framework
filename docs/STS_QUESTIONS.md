# NIST STS: OPEN QUESTIONS

##### 2022.06.25

### Ambiguity Noted Between NIST SP 800-22 Revision 1a Sections 2.14.4 (4) and 3.14

With regard to the Random Excursions Test, Section 2.14.4 (4) states, "Let J = the total number of zero crossings in S′, where a zero crossing is a value of zero in S′ that occurs after the starting zero. J is also the number of cycles in S′, where a cycle of S′ is a subsequence of S′ consisting of an occurrence of zero, followed by no-zero values, and ending with another zero. The ending zero in one cycle may be the beginning zero in another cycle. The number of cycles in S′ is the number of zero crossings. If J < 500, discontinue the test."

This suggests that in the case of J < 500, there are no test results, since the test was discontinued. The implementation of version 2.1.2 of the STS code reflects this, such that a test run is not counted as a test if J < 500, and therefore no test results are produced.

However, Section 3.1.4 states, "If J < max(0.005n,500), the randomness hypothesis is rejected."

This section is ambiguous. It's not precisely clear what "the randomness hypothesis" refers to, but if it's the same as the null hypothesis, then this section suggests that an insufficiently large value of J results in a test failure (the null hypothesis is rejected), not a test discard, as implied in both section 2.14.4 (4) and the STS source code.

This question also applies to Section 3.15 (Random Excursions Variant Test).

### Confidence Interval Conflicts with Significance Level When Evaluating Test Results

The confidence level criterion used to evaluate the STS test results often conflicts with the requirements of the significance level (alpha) default value (0.01).

### Must Each Bitstream Be Run Through Every STS Test?

With respect to testing bitstreams produced by an RBG, there is ambiguity regarding how STS testing should be carried out. Consider a test run consisting of the following 3 STS tests:

* Frequency
* Block frequency
* Runs

One approach is to test a single bitstream with the first of the 3 STS tests, followed by testing a new (different) bitstream with the second of the 3 STS tests, and so on, resulting in 3 bitstreams tested at the end of the test run, as shown below:

* Frequency test with bitstream 1
* Block frequency test with bitstream 2
* Runs test with bitstream 3

This is an example of how the entropy of an RBG via streaming might be tested.

Another approach is to test a single bitstream with _each_ of the 3 STS tests before moving on to test a new bitstream, as shown below:

* Frequency test with bitstream 1
* Block frequency test with bitstream 1
* Runs test with bitstream 1

* Frequency test with bitstream 2
* Block frequency test with bitstream 2
* Runs test with bitstream 2

* Frequency test with bitstream 3
* Block frequency test with bitstream 3
* Runs test with bitstream 3

This is an example of how the entropy of an RBG via files might be tested.

Is there a "correct" methodology to follow, or are the results of these two approaches sufficiently similar as to make no difference in the statistical evaluation of an RBG with the STS? NIST SP 800-22 Revision 1a does not appear to provide guidance on this question.

### Coding Error Identified in Discrete Fourier Transform Test Implementation

The main function in the original STS 2.1.2 "discreteFourierTransform.c" file is DiscreteFourierTransform at line 16, which accepts an argument “n” representing the length in bits of the bitstream to be tested. This value is used to allocate an array of doubles called “X” (refer to line 21). For example, if the bitstream length is 1000, then 1000 doubles are allocated.

An issue was uncovered when running valgrind/memcheck; it occurs at lines 41 and 42, reproduced here:

41	for ( i=0; i<n/2; i++ )
42		m[i+1] = sqrt(pow(X[2*i+1],2)+pow(X[2*i+2],2)); 

Using the previous example, if we allocated 1000 doubles for array X, then valid indices for this array fall within the range of 0 to 999 (inclusive).

Continuing the example, on line 41, the index “i” has a range of 0 to (1000/2 - 1), or 0 to 499 (inclusive).

If we evaluate line 42 when i is equal to 499, we get this:

m[500] = sqrt(pow(X[999],2)+pow(X[1000],2));

Evaluating X[1000] results in reading 8 bytes (the size of a double) beyond the end of the space allocated for X - this is what triggers the “Invalid read of size 8” in the valgrind/memcheck log.

The memcheck error can be corrected by changing the code at line 21 from:

	if ( ((X = (double*) calloc(n,sizeof(double))) == NULL) || 
	
To:

	if ( ((X = (double*) calloc((n + 1),sizeof(double))) == NULL) ||

This explicitly allocates an extra array element initialized to zero, thus preventing the invalid read.

HOWEVER: While this prevents the invalid memory access, it doesn’t address whether the intent of the test is properly implemented with respect to the documented behavior described in NIST SP 800-22 Rev. 1 sections 2.6 and 3.6.


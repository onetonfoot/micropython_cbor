import ucbor

def run():

    # generated using the python cbor2 library
    fixtures = {
        "int": (b'\x01', 1),
        "string": (b'cabc', "abc"),
        "bytes": (b'Cabc', b"abc"),
        "float64": (b'\xfb@\t\x1e\xb8Q\xeb\x85\x1f', 3.14),
        "float32": (b'\xfa@H\xf5\xc3', 3.14),
        "list": (b'\x83\x01\x02\x03', [1, 2, 3]),
        "nested_list": (b'\x83\x83\x82\x01\x02\x82\x03\x04\x05\x06\x07', [[[1,2], [3, 4], 5], 6, 7]),
        "dict": (b'\xa2dabcdcefgdhijkelmnop', {"hijk": "lmnop", "abcd": "efg"}),
        "big_mix": (b'\xa2cabc\x82\x83\x82\x01\x02\x82\x03\x04\x05\xa2cdefCghicjlk\x07clmn\x08', {"abc": [[[1,2], [3, 4], 5], {"def": b"ghi", "jlk": 7}], "lmn": 8}),
    }

    for key, val in fixtures.items():
        try:
            print("testing", key)
            print(val[0], "==", val[1])
            test = val[0]
            result = ucbor.loads(test)
            print(result)
            assert result == val[1]
            print("loads success")

            # check if dumps then loads is identical
            result = ucbor.loads(ucbor.dumps(result))
            assert result == val[1]
            print("dumps success\n\n")
        except AssertionError:
            print(key, " failed")


    print("check tuple encodes to array")
    assert ucbor.dumps((1, 2, 3)) == b'\x83\x01\x02\x03'
    print("success")
/****************************************************************************
**
** Copyright (C) 2021 Intel Corporation
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in
** all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
** THE SOFTWARE.
**
****************************************************************************/

#include "cbor.h"

#ifndef _
#  define _(msg)    msg
#endif

/**
 * \enum CborError
 * \ingroup CborGlobals
 * The CborError enum contains the possible error values used by the CBOR encoder and decoder.
 *
 * TinyCBOR functions report success by returning CborNoError, or one error
 * condition by returning one of the values below. One exception is the
 * out-of-memory condition (CborErrorOutOfMemory), which the functions for \ref
 * CborEncoding may report in bit-wise OR with other conditions.
 *
 * This technique allows code to determine whether the only error condition was
 * a lack of buffer space, which may not be a fatal condition if the buffer can
 * be resized. Additionally, the functions for \ref CborEncoding may continue
 * to be used even after CborErrorOutOfMemory is returned, and instead they
 * will simply calculate the extra space needed.
 *
 * \value CborNoError                   No error occurred
 * \omitvalue CborUnknownError
 * \value CborErrorUnknownLength        Request for the length of an array, map or string whose length is not provided in the CBOR stream
 * \value CborErrorAdvancePastEOF       Not enough data in the stream to decode item (decoding would advance past end of stream)
 * \value CborErrorIO                   An I/O error occurred, probably due to an out-of-memory situation
 * \value CborErrorGarbageAtEnd         Bytes exist past the end of the CBOR stream
 * \value CborErrorUnexpectedEOF        End of stream reached unexpectedly
 * \value CborErrorUnexpectedBreak      A CBOR break byte was found where not expected
 * \value CborErrorUnknownType          An unknown type (future extension to CBOR) was found in the stream
 * \value CborErrorIllegalType          An invalid type was found while parsing a chunked CBOR string
 * \value CborErrorIllegalNumber        An illegal initial byte (encoding unspecified additional information) was found
 * \value CborErrorIllegalSimpleType    An illegal encoding of a CBOR Simple Type of value less than 32 was found
 * \omitvalue CborErrorUnknownSimpleType
 * \omitvalue CborErrorUnknownTag
 * \omitvalue CborErrorInappropriateTagForType
 * \omitvalue CborErrorDuplicateObjectKeys
 * \value CborErrorInvalidUtf8TextString Illegal UTF-8 encoding found while parsing CBOR Text String
 * \value CborErrorTooManyItems         Too many items were added to CBOR map or array of pre-determined length
 * \value CborErrorTooFewItems          Too few items were added to CBOR map or array of pre-determined length
 * \value CborErrorDataTooLarge         Data item size exceeds TinyCBOR's implementation limits
 * \value CborErrorNestingTooDeep       Data item nesting exceeds TinyCBOR's implementation limits
 * \omitvalue CborErrorUnsupportedType
 * \value CborErrorJsonObjectKeyIsAggregate Conversion to JSON failed because the key in a map is a CBOR map or array
 * \value CborErrorJsonObjectKeyNotString Conversion to JSON failed because the key in a map is not a text string
 * \value CborErrorOutOfMemory          During CBOR encoding, the buffer provided is insufficient for encoding the data item;
 *                                      in other situations, TinyCBOR failed to allocate memory
 * \value CborErrorInternalError        An internal error occurred in TinyCBOR
 */

/**
 * \ingroup CborGlobals
 * Returns the error string corresponding to the CBOR error condition \a error.
 */
const char *cbor_error_string(CborError error)
{

    if (error == CborNoError){
        return "";
    } else if (error == CborUnknownError){
        return _("unknown error");
    } else if (error == CborErrorOutOfMemory){
        return _("out of memory/need more memory");
    } else if (error == CborErrorUnknownLength){
        return _("unknown length (attempted to get the length of a map/array/string of indeterminate length");
    } else if (error == CborErrorAdvancePastEOF ) {
        return _("attempted to advance past EOF");
    } else if (error == CborErrorIO) {
        return _("I/O error");
    } else if (error == CborErrorGarbageAtEnd ){
        return _("garbage after the end of the content");
    } else if (error == CborErrorUnexpectedEOF){
        return _("unexpected end of data");
    } else if (error == CborErrorUnexpectedBreak) {
        return _("unexpected 'break' byte");
    } else if (error == CborErrorUnknownType) {
        return _("illegal byte (encodes future extension type)");
    } else if (error == CborErrorIllegalType){
        return _("mismatched string type in chunked string");
    } else if (error == CborErrorIllegalNumber){
        return _("illegal initial byte (encodes unspecified additional information)");
    } else if (error == CborErrorIllegalSimpleType) {
        return _("illegal encoding of simple type smaller than 32");
    } else if (error == CborErrorNoMoreStringChunks){
        return _("no more byte or text strings available");
    } else if (error == CborErrorUnknownSimpleType){
        return _("unknown simple type");
    } else if (error == CborErrorUnknownTag){
        return _("unknown tag");
    } else if (error == CborErrorInappropriateTagForType){
        return _("inappropriate tag for type");
    } else if (error == CborErrorDuplicateObjectKeys){
        return _("duplicate keys in object");
    } else if (error == CborErrorInvalidUtf8TextString){
        return _("invalid UTF-8 content in string");
    } else if (error == CborErrorExcludedType) {
        return _("excluded type found");
    } else if (error == CborErrorImproperValue){
        return _("excluded value found");
    } else if (error == CborErrorOverlongEncoding){
        return _("value encoded in non-canonical form");
    } else if (error == CborErrorMapKeyNotString || error == CborErrorJsonObjectKeyNotString) {
        return _("key in map is not a string");
    } else if (error == CborErrorMapNotSorted){
        return _("map is not sorted");
    } else if (error ==  CborErrorMapKeysNotUnique) {
        return _("map keys are not unique");
    } else if ( error == CborErrorTooManyItems) {
        return _("too many items added to encoder");
    } else if ( error == CborErrorDataTooLarge) {
        return _("internal error: data too large");
    } else if ( error == CborErrorNestingTooDeep) {
        return _("internal error: too many nested containers found in recursive function");
    } else if ( error == CborErrorUnsupportedType) {
        return _("unsupported type");
    } else if ( error == CborErrorUnimplementedValidation) {
        return _("validation not implemented for the current parser state");
    } else if ( error == CborErrorJsonObjectKeyIsAggregate) {
        return _("conversion to JSON failed: key in object is an array or map");
    } else if ( error == CborErrorJsonNotImplemented) {
        return _("conversion to JSON failed: open_memstream unavailable");
    } else if ( error == CborErrorInternalError) {
        return _("internal error");
    } else {
        return cbor_error_string(CborUnknownError);
    }
}

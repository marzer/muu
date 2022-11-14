// This file is a part of muu and is subject to the the terms of the MIT license.
// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
// SPDX-License-Identifier: MIT
#pragma once

#include "../fwd.h"
MUU_DISABLE_WARNINGS;
#include <limits>
MUU_ENABLE_WARNINGS;
#include "header_start.h"
MUU_PUSH_PRECISE_MATH;

#if MUU_GCC && MUU_HAS_FLOAT128
	#pragma GCC system_header // float128 literals cause a warning in GCC that can't be silenced otherwise :(
#endif

namespace muu
{
	namespace impl
	{
		//------------- helpers
		/// \cond

		template <typename T, intmax_t Base, intmax_t Exponent>
		struct power_helper_
		{
			static constexpr T value = T{ Base } * power_helper_<T, Base, Exponent - 1>::value;
		};
		template <typename T, intmax_t Exponent>
		struct power_helper_<T, Exponent, 0>
		{
			static constexpr T value = T{ 1 };
		};
		template <typename T, intmax_t Base, intmax_t Exponent>
		struct power_
		{
			static constexpr intmax_t abs_exponent = Exponent < 0 ? -Exponent : Exponent;
			static constexpr T value = Exponent < 0 ? T{ 1 } / T{ power_helper_<T, Base, abs_exponent>::value }
													: T{ power_helper_<T, Base, abs_exponent>::value };
		};

		/// \endcond

		//------------- standalone 'trait' constant classes

		template <typename T>
		struct integer_limits
		{
			/// \name Limits
			/// @{

			/// \brief The lowest representable 'normal' value (equivalent to std::numeric_limits::lowest()).
			static constexpr T lowest = std::numeric_limits<T>::lowest();

			/// \brief The highest representable 'normal' value (equivalent to std::numeric_limits::max()).
			static constexpr T highest = std::numeric_limits<T>::max();

			/// @}
		};

		/// \cond

#if MUU_HAS_INT128
		template <>
		struct integer_limits<int128_t>
		{
			static constexpr int128_t highest =
				static_cast<int128_t>((uint128_t{ 1u } << ((__SIZEOF_INT128__ * CHAR_BIT) - 1)) - 1);
			static constexpr int128_t lowest = -highest - int128_t{ 1 };
		};
		template <>
		struct integer_limits<uint128_t>
		{
			static constexpr uint128_t lowest  = uint128_t{};
			static constexpr uint128_t highest = (2u * static_cast<uint128_t>(integer_limits<int128_t>::highest)) + 1u;
		};
#endif // MUU_HAS_INT128

#if MUU_HAS_FLOAT128
		template <>
		struct integer_limits<float128_t>
		{
			static constexpr float128_t highest = 1.18973149535723176508575932662800702e4932q;
			static constexpr float128_t lowest	= -highest;
		};
#endif

		/// \endcond

		template <typename T>
		struct integer_positive_constants
		{
			/// \name Integers
			/// @{

			static constexpr T zero		   = T{ 0 };   ///< `0`
			static constexpr T one		   = T{ 1 };   ///< `1`
			static constexpr T two		   = T{ 2 };   ///< `2`
			static constexpr T three	   = T{ 3 };   ///< `3`
			static constexpr T four		   = T{ 4 };   ///< `4`
			static constexpr T five		   = T{ 5 };   ///< `5`
			static constexpr T six		   = T{ 6 };   ///< `6`
			static constexpr T seven	   = T{ 7 };   ///< `7`
			static constexpr T eight	   = T{ 8 };   ///< `8`
			static constexpr T nine		   = T{ 9 };   ///< `9`
			static constexpr T ten		   = T{ 10 };  ///< `10`
			static constexpr T one_hundred = T{ 100 }; ///< `100`

			/// @}
		};

		template <typename T>
		struct floating_point_traits
		{
			/// \name Floating-point
			/// @{

			/// \brief The number of significand (mantissa) digits.
			static constexpr int significand_digits = std::numeric_limits<T>::digits;

			/// \brief The number of significant decimal digits that can be exactly represented.
			static constexpr int decimal_digits = std::numeric_limits<T>::digits10;

			/// \brief The default epsilon used by #approx_equal.
			static constexpr T default_epsilon = T{ 10 } * power_<T, 10, -std::numeric_limits<T>::digits10>::value;

			/// @}
		};

		/// \cond

#if MUU_HAS_FLOAT16
		template <>
		struct floating_point_traits<_Float16>
		{
			static constexpr int significand_digits	  = 11;
			static constexpr int decimal_digits		  = 3;
			static constexpr _Float16 default_epsilon = static_cast<_Float16>(0.001);
		};
#endif

#if MUU_HAS_FLOAT128
		template <>
		struct floating_point_traits<float128_t>
		{
			static constexpr int significand_digits = __FLT128_MANT_DIG__;
			static constexpr int decimal_digits		= __FLT128_DIG__;
			static constexpr float128_t default_epsilon =
				float128_t{ 10 } * power_<float128_t, 10, -__FLT128_DIG__>::value;
		};
#endif

		/// \endcond

		template <typename T>
		struct floating_point_special_constants
		{
			/// \name Floating-point
			/// @{

			static constexpr T nan			 = std::numeric_limits<T>::quiet_NaN();		///< Not-A-Number (quiet)
			static constexpr T signaling_nan = std::numeric_limits<T>::signaling_NaN(); ///< Not-A-Number (signalling)
			static constexpr T infinity		 = std::numeric_limits<T>::infinity();		///< Positive infinity
			static constexpr T negative_infinity = -infinity;							///< Negative infinity
			static constexpr T negative_zero	 = -T{};								///< `-0.0`

			/// @}
		};

		template <typename T>
		struct floating_point_named_constants
		{
			/// \name Irrational numbers and ratios
			/// @{

			static constexpr T one_over_two			  = T(0.500000000000000000000L); ///< `1 / 2`
			static constexpr T one_over_three		  = T(0.333333333333333333333L); ///< `1 / 3`
			static constexpr T one_over_four		  = T(0.250000000000000000000L); ///< `1 / 4`
			static constexpr T one_over_five		  = T(0.200000000000000000000L); ///< `1 / 5`
			static constexpr T one_over_six			  = T(0.166666666666666666667L); ///< `1 / 6`
			static constexpr T one_over_seven		  = T(0.142857142857142857143L); ///< `1 / 7`
			static constexpr T one_over_eight		  = T(0.125000000000000000000L); ///< `1 / 8`
			static constexpr T sqrt_two				  = T(1.414213562373095048802L); ///< `sqrt(2)`
			static constexpr T one_over_sqrt_two	  = T(0.707106781186547524401L); ///< `1 / sqrt(2)`
			static constexpr T two_over_three		  = T(0.666666666666666666667L); ///< `2 / 3`
			static constexpr T two_over_five		  = T(0.400000000000000000000L); ///< `2 / 5`
			static constexpr T two_over_seven		  = T(0.285714285714285714286L); ///< `2 / 7`
			static constexpr T sqrt_three			  = T(1.732050807568877293527L); ///< `sqrt(3)`
			static constexpr T one_over_sqrt_three	  = T(0.577350269189625764509L); ///< `1 / sqrt(3)`
			static constexpr T three_over_two		  = T(1.500000000000000000000L); ///< `3 / 2`
			static constexpr T three_over_four		  = T(0.750000000000000000000L); ///< `3 / 4`
			static constexpr T three_over_five		  = T(0.600000000000000000000L); ///< `3 / 5`
			static constexpr T three_over_seven		  = T(0.428571428571428571429L); ///< `3 / 7`
			static constexpr T three_over_eight		  = T(0.375000000000000000000L); ///< `3 / 8`
			static constexpr T sqrt_four			  = T(2.000000000000000000000L); ///< `sqrt(4)`
			static constexpr T four_over_three		  = T(1.333333333333333333333L); ///< `4 / 3`
			static constexpr T four_over_five		  = T(0.800000000000000000000L); ///< `4 / 5`
			static constexpr T four_over_seven		  = T(0.571428571428571428571L); ///< `4 / 7`
			static constexpr T pi					  = T(3.141592653589793238463L); ///< `pi`
			static constexpr T one_over_pi			  = T(0.318309886183790671538L); ///< `1 / pi`
			static constexpr T sqrt_pi				  = T(1.772453850905516027298L); ///< `sqrt(pi)`
			static constexpr T one_over_sqrt_pi		  = T(0.564189583547756286948L); ///< `1 / sqrt(pi)`
			static constexpr T pi_over_two			  = T(1.570796326794896619231L); ///< `pi / 2`
			static constexpr T pi_over_three		  = T(1.047197551196597746154L); ///< `pi / 3`
			static constexpr T pi_over_four			  = T(0.785398163397448309616L); ///< `pi / 4`
			static constexpr T pi_over_five			  = T(0.628318530717958647693L); ///< `pi / 5`
			static constexpr T pi_over_six			  = T(0.523598775598298873077L); ///< `pi / 6`
			static constexpr T pi_over_seven		  = T(0.448798950512827605495L); ///< `pi / 7`
			static constexpr T pi_over_eight		  = T(0.392699081698724154808L); ///< `pi / 8`
			static constexpr T two_over_pi			  = T(0.636619772367581343076L); ///< `2 / pi`
			static constexpr T three_over_pi		  = T(0.954929658551372014613L); ///< `3 / pi`
			static constexpr T four_over_pi			  = T(1.273239544735162686151L); ///< `4 / pi`
			static constexpr T five_over_pi			  = T(1.591549430918953357689L); ///< `5 / pi`
			static constexpr T six_over_pi			  = T(1.909859317102744029227L); ///< `6 / pi`
			static constexpr T seven_over_pi		  = T(2.228169203286534700764L); ///< `7 / pi`
			static constexpr T eight_over_pi		  = T(2.546479089470325372302L); ///< `8 / pi`
			static constexpr T two_pi				  = T(6.283185307179586476925L); ///< `2 * pi`
			static constexpr T one_over_two_pi		  = T(0.159154943091895335769L); ///< `1 / (2 * pi)`
			static constexpr T sqrt_two_pi			  = T(2.506628274631000502416L); ///< `sqrt(2 * pi)`
			static constexpr T one_over_sqrt_two_pi	  = T(0.398942280401432677940L); ///< `1 / sqrt(2 * pi)`
			static constexpr T two_pi_over_three	  = T(2.094395102393195492308L); ///< `(2 * pi) / 3`
			static constexpr T two_pi_over_five		  = T(1.256637061435917295385L); ///< `(2 * pi) / 5`
			static constexpr T two_pi_over_seven	  = T(0.897597901025655210989L); ///< `(2 * pi) / 7`
			static constexpr T three_over_two_pi	  = T(0.477464829275686007307L); ///< `3 / (2 * pi)`
			static constexpr T five_over_two_pi		  = T(0.795774715459476678844L); ///< `5 / (2 * pi)`
			static constexpr T seven_over_two_pi	  = T(1.114084601643267350382L); ///< `7 / (2 * pi)`
			static constexpr T three_pi				  = T(9.424777960769379715388L); ///< `3 * pi`
			static constexpr T one_over_three_pi	  = T(0.106103295394596890513L); ///< `1 / (3 * pi)`
			static constexpr T sqrt_three_pi		  = T(3.069980123839465465439L); ///< `sqrt(3 * pi)`
			static constexpr T one_over_sqrt_three_pi = T(0.325735007935279947724L); ///< `1 / sqrt(3 * pi)`
			static constexpr T three_pi_over_two	  = T(4.712388980384689857694L); ///< `(3 * pi) / 2`
			static constexpr T three_pi_over_four	  = T(2.356194490192344928847L); ///< `(3 * pi) / 4`
			static constexpr T three_pi_over_five	  = T(1.884955592153875943078L); ///< `(3 * pi) / 5`
			static constexpr T three_pi_over_seven	  = T(1.346396851538482816484L); ///< `(3 * pi) / 7`
			static constexpr T three_pi_over_eight	  = T(1.178097245096172464423L); ///< `(3 * pi) / 8`
			static constexpr T two_over_three_pi	  = T(0.212206590789193781025L); ///< `2 / (3 * pi)`
			static constexpr T four_over_three_pi	  = T(0.424413181578387562050L); ///< `4 / (3 * pi)`
			static constexpr T five_over_three_pi	  = T(0.530516476972984452563L); ///< `5 / (3 * pi)`
			static constexpr T seven_over_three_pi	  = T(0.742723067762178233588L); ///< `7 / (3 * pi)`
			static constexpr T eight_over_three_pi	  = T(0.848826363156775124101L); ///< `8 / (3 * pi)`
			static constexpr T e					  = T(2.718281828459045534885L); ///< `e`
			static constexpr T one_over_e			  = T(0.367879441171442281059L); ///< `1 / e`
			static constexpr T sqrt_e				  = T(1.648721270700128237684L); ///< `sqrt(e)`
			static constexpr T one_over_sqrt_e		  = T(0.606530659712633390187L); ///< `1 / sqrt(e)`
			static constexpr T e_over_two			  = T(1.359140914229522767442L); ///< `e / 2`
			static constexpr T e_over_three			  = T(0.906093942819681844962L); ///< `e / 3`
			static constexpr T e_over_four			  = T(0.679570457114761383721L); ///< `e / 4`
			static constexpr T e_over_five			  = T(0.543656365691809106977L); ///< `e / 5`
			static constexpr T e_over_six			  = T(0.453046971409840922481L); ///< `e / 6`
			static constexpr T e_over_seven			  = T(0.388325975494149362126L); ///< `e / 7`
			static constexpr T e_over_eight			  = T(0.339785228557380691861L); ///< `e / 8`
			static constexpr T two_over_e			  = T(0.735758882342884562119L); ///< `2 / e`
			static constexpr T three_over_e			  = T(1.103638323514326843178L); ///< `3 / e`
			static constexpr T four_over_e			  = T(1.471517764685769124237L); ///< `4 / e`
			static constexpr T five_over_e			  = T(1.839397205857211405296L); ///< `5 / e`
			static constexpr T six_over_e			  = T(2.207276647028653686356L); ///< `6 / e`
			static constexpr T seven_over_e			  = T(2.575156088200095967415L); ///< `7 / e`
			static constexpr T eight_over_e			  = T(2.943035529371538248474L); ///< `8 / e`
			static constexpr T phi					  = T(1.618033988749894848205L); ///< `phi`
			static constexpr T one_over_phi			  = T(0.618033988749894848205L); ///< `1 / phi`
			static constexpr T sqrt_phi				  = T(1.272019649514068964252L); ///< `sqrt(phi)`
			static constexpr T one_over_sqrt_phi	  = T(0.786151377757423286070L); ///< `1 / sqrt(phi)`
			static constexpr T phi_over_two			  = T(0.809016994374947424102L); ///< `phi / 2`
			static constexpr T phi_over_three		  = T(0.539344662916631616068L); ///< `phi / 3`
			static constexpr T phi_over_four		  = T(0.404508497187473712051L); ///< `phi / 4`
			static constexpr T phi_over_five		  = T(0.323606797749978969641L); ///< `phi / 5`
			static constexpr T phi_over_six			  = T(0.269672331458315808034L); ///< `phi / 6`
			static constexpr T phi_over_seven		  = T(0.231147712678556406886L); ///< `phi / 7`
			static constexpr T phi_over_eight		  = T(0.202254248593736856026L); ///< `phi / 8`
			static constexpr T two_over_phi			  = T(1.236067977499789696409L); ///< `2 / phi`
			static constexpr T three_over_phi		  = T(1.854101966249684544614L); ///< `2 / phi`
			static constexpr T four_over_phi		  = T(2.472135954999579392818L); ///< `2 / phi`
			static constexpr T five_over_phi		  = T(3.090169943749474241023L); ///< `2 / phi`
			static constexpr T six_over_phi			  = T(3.708203932499369089228L); ///< `2 / phi`
			static constexpr T seven_over_phi		  = T(4.326237921249263937432L); ///< `2 / phi`
			static constexpr T eight_over_phi		  = T(4.944271909999158785637L); ///< `2 / phi`
			static constexpr T one_over_ten			  = T(0.100000000000000000000L); ///< `1 / 10`
			static constexpr T one_over_one_hundred	  = T(0.010000000000000000000L); ///< `1 / 100`
			static constexpr T one_over_one_thousand  = T(0.001000000000000000000L); ///< `1 / 1000`

			/// @}

			/// \name Conversions
			/// @{

			/// \brief  Conversion factor for converting degrees into radians.
			static constexpr T degrees_to_radians = T(0.017453292519943295769L);
			/// \brief  Conversion factor for converting radians into degrees.
			static constexpr T radians_to_degrees = T(57.295779513082320876798L);

			/// @}
		};

		/// \cond
#if MUU_HAS_FLOAT128
		template <>
		struct floating_point_named_constants<float128_t>
		{
			static constexpr float128_t one_over_two		   = 0.500000000000000000000000000000000000q;
			static constexpr float128_t one_over_three		   = 0.333333333333333333333333333333333333q;
			static constexpr float128_t one_over_four		   = 0.250000000000000000000000000000000000q;
			static constexpr float128_t one_over_five		   = 0.200000000000000000000000000000000000q;
			static constexpr float128_t one_over_six		   = 0.166666666666666666666666666666666667q;
			static constexpr float128_t one_over_seven		   = 0.142857142857142857142857142857142857q;
			static constexpr float128_t one_over_eight		   = 0.125000000000000000000000000000000000q;
			static constexpr float128_t sqrt_two			   = 1.414213562373095048801688724209698079q;
			static constexpr float128_t one_over_sqrt_two	   = 0.707106781186547524400844362104849039q;
			static constexpr float128_t two_over_three		   = 0.666666666666666666666666666666666667q;
			static constexpr float128_t two_over_five		   = 0.400000000000000000000000000000000000q;
			static constexpr float128_t two_over_seven		   = 0.285714285714285714285714285714285714q;
			static constexpr float128_t sqrt_three			   = 1.732050807568877293527446341505872367q;
			static constexpr float128_t one_over_sqrt_three	   = 0.577350269189625764509148780501957456q;
			static constexpr float128_t three_over_two		   = 1.500000000000000000000000000000000000q;
			static constexpr float128_t three_over_four		   = 0.750000000000000000000000000000000000q;
			static constexpr float128_t three_over_five		   = 0.600000000000000000000000000000000000q;
			static constexpr float128_t three_over_seven	   = 0.428571428571428571428571428571428571q;
			static constexpr float128_t three_over_eight	   = 0.375000000000000000000000000000000000q;
			static constexpr float128_t sqrt_four			   = 2.000000000000000000000000000000000000q;
			static constexpr float128_t four_over_three		   = 1.333333333333333333333333333333333333q;
			static constexpr float128_t four_over_five		   = 0.800000000000000000000000000000000000q;
			static constexpr float128_t four_over_seven		   = 0.571428571428571428571428571428571429q;
			static constexpr float128_t pi					   = 3.141592653589793238462643383279502884q;
			static constexpr float128_t one_over_pi			   = 0.318309886183790671537767526745028724q;
			static constexpr float128_t sqrt_pi				   = 1.772453850905516027298167483341145183q;
			static constexpr float128_t one_over_sqrt_pi	   = 0.564189583547756286948079451560772586q;
			static constexpr float128_t pi_over_two			   = 1.570796326794896619231321691639751442q;
			static constexpr float128_t pi_over_three		   = 1.047197551196597746154214461093167628q;
			static constexpr float128_t pi_over_four		   = 0.785398163397448309615660845819875721q;
			static constexpr float128_t pi_over_five		   = 0.628318530717958647692528676655900577q;
			static constexpr float128_t pi_over_six			   = 0.523598775598298873077107230546583814q;
			static constexpr float128_t pi_over_seven		   = 0.448798950512827605494663340468500412q;
			static constexpr float128_t pi_over_eight		   = 0.392699081698724154807830422909937861q;
			static constexpr float128_t two_over_pi			   = 0.636619772367581343075535053490057448q;
			static constexpr float128_t three_over_pi		   = 0.954929658551372014613302580235086172q;
			static constexpr float128_t four_over_pi		   = 1.273239544735162686151070106980114896q;
			static constexpr float128_t five_over_pi		   = 1.591549430918953357688837633725143620q;
			static constexpr float128_t six_over_pi			   = 1.909859317102744029226605160470172344q;
			static constexpr float128_t seven_over_pi		   = 2.228169203286534700764372687215201068q;
			static constexpr float128_t eight_over_pi		   = 2.546479089470325372302140213960229793q;
			static constexpr float128_t two_pi				   = 6.283185307179586476925286766559005768q;
			static constexpr float128_t one_over_two_pi		   = 0.159154943091895335768883763372514362q;
			static constexpr float128_t sqrt_two_pi			   = 2.506628274631000502415765284811045253q;
			static constexpr float128_t one_over_sqrt_two_pi   = 0.398942280401432677939946059934381868q;
			static constexpr float128_t two_pi_over_three	   = 2.094395102393195492308428922186335256q;
			static constexpr float128_t two_pi_over_five	   = 1.256637061435917295385057353311801154q;
			static constexpr float128_t two_pi_over_seven	   = 0.897597901025655210989326680937000824q;
			static constexpr float128_t three_over_two_pi	   = 0.477464829275686007306651290117543086q;
			static constexpr float128_t five_over_two_pi	   = 0.795774715459476678844418816862571810q;
			static constexpr float128_t seven_over_two_pi	   = 1.114084601643267350382186343607600534q;
			static constexpr float128_t three_pi			   = 9.424777960769379715387930149838508653q;
			static constexpr float128_t one_over_three_pi	   = 0.106103295394596890512589175581676241q;
			static constexpr float128_t sqrt_three_pi		   = 3.069980123839465465438654874667794582q;
			static constexpr float128_t one_over_sqrt_three_pi = 0.325735007935279947724256415225564670q;
			static constexpr float128_t three_pi_over_two	   = 4.712388980384689857693965074919254326q;
			static constexpr float128_t three_pi_over_four	   = 2.356194490192344928846982537459627163q;
			static constexpr float128_t three_pi_over_five	   = 1.884955592153875943077586029967701731q;
			static constexpr float128_t three_pi_over_seven	   = 1.346396851538482816483990021405501236q;
			static constexpr float128_t three_pi_over_eight	   = 1.178097245096172464423491268729813582q;
			static constexpr float128_t two_over_three_pi	   = 0.212206590789193781025178351163352483q;
			static constexpr float128_t four_over_three_pi	   = 0.424413181578387562050356702326704965q;
			static constexpr float128_t five_over_three_pi	   = 0.530516476972984452562945877908381207q;
			static constexpr float128_t seven_over_three_pi	   = 0.742723067762178233588124229071733689q;
			static constexpr float128_t eight_over_three_pi	   = 0.848826363156775124100713404653409931q;
			static constexpr float128_t e					   = 2.718281828459045534884808148490265012q;
			static constexpr float128_t one_over_e			   = 0.367879441171442281059287928010393142q;
			static constexpr float128_t sqrt_e				   = 1.648721270700128237684053351021451524q;
			static constexpr float128_t one_over_sqrt_e		   = 0.606530659712633390187322401455485737q;
			static constexpr float128_t e_over_two			   = 1.359140914229522767442404074245132506q;
			static constexpr float128_t e_over_three		   = 0.906093942819681844961602716163421671q;
			static constexpr float128_t e_over_four			   = 0.679570457114761383721202037122566253q;
			static constexpr float128_t e_over_five			   = 0.543656365691809106976961629698053002q;
			static constexpr float128_t e_over_six			   = 0.453046971409840922480801358081710835q;
			static constexpr float128_t e_over_seven		   = 0.388325975494149362126401164070037859q;
			static constexpr float128_t e_over_eight		   = 0.339785228557380691860601018561283126q;
			static constexpr float128_t two_over_e			   = 0.735758882342884562118575856020786284q;
			static constexpr float128_t three_over_e		   = 1.103638323514326843177863784031179427q;
			static constexpr float128_t four_over_e			   = 1.471517764685769124237151712041572569q;
			static constexpr float128_t five_over_e			   = 1.839397205857211405296439640051965711q;
			static constexpr float128_t six_over_e			   = 2.207276647028653686355727568062358853q;
			static constexpr float128_t seven_over_e		   = 2.575156088200095967415015496072751996q;
			static constexpr float128_t eight_over_e		   = 2.943035529371538248474303424083145138q;
			static constexpr float128_t phi					   = 1.618033988749894848204586834365638118q;
			static constexpr float128_t one_over_phi		   = 0.618033988749894848204586834365638118q;
			static constexpr float128_t sqrt_phi			   = 1.272019649514068964252422461737491492q;
			static constexpr float128_t one_over_sqrt_phi	   = 0.786151377757423286069558585842958930q;
			static constexpr float128_t phi_over_two		   = 0.809016994374947424102293417182819059q;
			static constexpr float128_t phi_over_three		   = 0.539344662916631616068195611455212706q;
			static constexpr float128_t phi_over_four		   = 0.404508497187473712051146708591409529q;
			static constexpr float128_t phi_over_five		   = 0.323606797749978969640917366873127624q;
			static constexpr float128_t phi_over_six		   = 0.269672331458315808034097805727606353q;
			static constexpr float128_t phi_over_seven		   = 0.231147712678556406886369547766519731q;
			static constexpr float128_t phi_over_eight		   = 0.202254248593736856025573354295704765q;
			static constexpr float128_t two_over_phi		   = 1.236067977499789696409173668731276235q;
			static constexpr float128_t three_over_phi		   = 1.854101966249684544613760503096914353q;
			static constexpr float128_t four_over_phi		   = 2.472135954999579392818347337462552471q;
			static constexpr float128_t five_over_phi		   = 3.090169943749474241022934171828190589q;
			static constexpr float128_t six_over_phi		   = 3.708203932499369089227521006193828706q;
			static constexpr float128_t seven_over_phi		   = 4.326237921249263937432107840559466824q;
			static constexpr float128_t eight_over_phi		   = 4.944271909999158785636694674925104942q;
			static constexpr float128_t one_over_ten		   = 0.100000000000000000000000000000000000q;
			static constexpr float128_t one_over_one_hundred   = 0.010000000000000000000000000000000000q;
			static constexpr float128_t one_over_one_thousand  = 0.001000000000000000000000000000000000q;
			static constexpr float128_t degrees_to_radians	   = 0.017453292519943295769236907684886127q;
			static constexpr float128_t radians_to_degrees	   = 57.295779513082320876798154814105170332q;
		};
#endif
		/// \endcond

		//-------------  constant class aggregates

		template <typename T>
		struct ascii_character_constants : integer_limits<T>, integer_positive_constants<T>
		{
			/// \name Control characters
			/// @{

			static constexpr T backspace = T{ 8 };	 ///< The backspace character.
			static constexpr T escape	 = T{ 27 };	 ///< `ESC`
			static constexpr T del		 = T{ 127 }; ///< `DEL`

			/// @}

			/// \name Whitespace
			/// @{

			static constexpr T tab			   = T{ 9 };  ///< `\t`
			static constexpr T new_line		   = T{ 10 }; ///< `\n`
			static constexpr T vertical_tab	   = T{ 11 }; ///< `\v`
			static constexpr T form_feed	   = T{ 12 }; ///< `\f`
			static constexpr T carriage_return = T{ 13 }; ///< `\r`
			static constexpr T space		   = T{ 32 }; ///< `&nbsp;` (space)

			/// @}

			/// \name Punctuation
			/// @{

			static constexpr T exclamation_mark	 = T{ 33 }; ///< `!`
			static constexpr T quote			 = T{ 34 }; ///< `"`
			static constexpr T number_sign		 = T{ 35 }; ///< `#`
			static constexpr T dollar_sign		 = T{ 36 }; ///< `$`
			static constexpr T percent			 = T{ 37 }; ///< `%`
			static constexpr T ampersand		 = T{ 38 }; ///< `&amp;`
			static constexpr T apostrophe		 = T{ 39 }; ///< `&apos;`
			static constexpr T left_parenthesis	 = T{ 40 }; ///< `(`
			static constexpr T right_parenthesis = T{ 41 }; ///< `)`
			static constexpr T asterisk			 = T{ 42 }; ///< `*`
			static constexpr T plus				 = T{ 43 }; ///< `+`
			static constexpr T comma			 = T{ 44 }; ///< `,`
			static constexpr T hyphen			 = T{ 45 }; ///< `-`
			static constexpr T period			 = T{ 46 }; ///< `.`
			static constexpr T forward_slash	 = T{ 47 }; ///< `/`

			static constexpr T colon		 = T{ 58 }; ///< `:`
			static constexpr T semi_colon	 = T{ 59 }; ///< `;`
			static constexpr T less_than	 = T{ 60 }; ///< `&lt;`
			static constexpr T equal		 = T{ 61 }; ///< `=`
			static constexpr T greater_than	 = T{ 62 }; ///< `&gt;`
			static constexpr T question_mark = T{ 63 }; ///< `?`
			static constexpr T at			 = T{ 64 }; ///< `@`

			/// \name Digits
			/// @{

			static constexpr T digit_0 = T{ 48 }; ///< `0`
			static constexpr T digit_1 = T{ 49 }; ///< `1`
			static constexpr T digit_2 = T{ 50 }; ///< `2`
			static constexpr T digit_3 = T{ 51 }; ///< `3`
			static constexpr T digit_4 = T{ 52 }; ///< `4`
			static constexpr T digit_5 = T{ 53 }; ///< `5`
			static constexpr T digit_6 = T{ 54 }; ///< `6`
			static constexpr T digit_7 = T{ 55 }; ///< `7`
			static constexpr T digit_8 = T{ 56 }; ///< `8`
			static constexpr T digit_9 = T{ 57 }; ///< `9`

			/// @}

			/// \name Letters
			/// @{

			static constexpr T letter_A = T{ 65 }; ///< `A`
			static constexpr T letter_B = T{ 66 }; ///< `B`
			static constexpr T letter_C = T{ 67 }; ///< `C`
			static constexpr T letter_D = T{ 68 }; ///< `D`
			static constexpr T letter_E = T{ 69 }; ///< `E`
			static constexpr T letter_F = T{ 70 }; ///< `F`
			static constexpr T letter_G = T{ 71 }; ///< `G`
			static constexpr T letter_H = T{ 72 }; ///< `H`
			static constexpr T letter_I = T{ 73 }; ///< `I`
			static constexpr T letter_J = T{ 74 }; ///< `J`
			static constexpr T letter_K = T{ 75 }; ///< `K`
			static constexpr T letter_L = T{ 76 }; ///< `L`
			static constexpr T letter_M = T{ 77 }; ///< `M`
			static constexpr T letter_N = T{ 78 }; ///< `N`
			static constexpr T letter_O = T{ 79 }; ///< `O`
			static constexpr T letter_P = T{ 80 }; ///< `P`
			static constexpr T letter_Q = T{ 81 }; ///< `Q`
			static constexpr T letter_R = T{ 82 }; ///< `R`
			static constexpr T letter_S = T{ 83 }; ///< `S`
			static constexpr T letter_T = T{ 84 }; ///< `T`
			static constexpr T letter_U = T{ 85 }; ///< `U`
			static constexpr T letter_V = T{ 86 }; ///< `V`
			static constexpr T letter_W = T{ 87 }; ///< `W`
			static constexpr T letter_X = T{ 88 }; ///< `X`
			static constexpr T letter_Y = T{ 89 }; ///< `Y`
			static constexpr T letter_Z = T{ 90 }; ///< `Z`

			static constexpr T letter_a = T{ 97 };	///< `a`
			static constexpr T letter_b = T{ 98 };	///< `b`
			static constexpr T letter_c = T{ 99 };	///< `c`
			static constexpr T letter_d = T{ 100 }; ///< `d`
			static constexpr T letter_e = T{ 101 }; ///< `e`
			static constexpr T letter_f = T{ 102 }; ///< `f`
			static constexpr T letter_g = T{ 103 }; ///< `g`
			static constexpr T letter_h = T{ 104 }; ///< `h`
			static constexpr T letter_i = T{ 105 }; ///< `i`
			static constexpr T letter_j = T{ 106 }; ///< `j`
			static constexpr T letter_k = T{ 107 }; ///< `k`
			static constexpr T letter_l = T{ 108 }; ///< `l`
			static constexpr T letter_m = T{ 109 }; ///< `m`
			static constexpr T letter_n = T{ 110 }; ///< `n`
			static constexpr T letter_o = T{ 111 }; ///< `o`
			static constexpr T letter_p = T{ 112 }; ///< `p`
			static constexpr T letter_q = T{ 113 }; ///< `q`
			static constexpr T letter_r = T{ 114 }; ///< `r`
			static constexpr T letter_s = T{ 115 }; ///< `s`
			static constexpr T letter_t = T{ 116 }; ///< `t`
			static constexpr T letter_u = T{ 117 }; ///< `u`
			static constexpr T letter_v = T{ 118 }; ///< `v`
			static constexpr T letter_w = T{ 119 }; ///< `w`
			static constexpr T letter_x = T{ 120 }; ///< `x`
			static constexpr T letter_y = T{ 121 }; ///< `y`
			static constexpr T letter_z = T{ 122 }; ///< `z`

			/// @}

			/// \name Punctuation
			/// @{

			static constexpr T left_square_bracket	= T{ 91 }; ///< `[`
			static constexpr T back_slash			= T{ 92 }; ///< `\\`
			static constexpr T right_square_bracket = T{ 93 }; ///< `]`
			static constexpr T hat					= T{ 94 }; ///< `^`
			static constexpr T underscore			= T{ 95 }; ///< `_`
			static constexpr T backtick				= T{ 96 }; ///< `[htmlentity #96]`

			static constexpr T left_brace  = T{ 123 }; ///< `{`
			static constexpr T bar		   = T{ 124 }; ///< `|`
			static constexpr T right_brace = T{ 125 }; ///< `}`
			static constexpr T tilde	   = T{ 126 }; ///< `~`

			/// @}
		};

		template <typename T>
		struct floating_point_constants : integer_limits<T>,
										  integer_positive_constants<T>,
										  floating_point_traits<T>,
										  floating_point_special_constants<T>,
										  floating_point_named_constants<T>
		{};

		template <typename T>
		struct unsigned_integral_constants : integer_limits<T>, integer_positive_constants<T>
		{};

		template <typename T>
		struct signed_integral_constants : integer_limits<T>, integer_positive_constants<T>
		{};
	}

	/// \addtogroup		constants
	/// @{

	/// \brief	`float` constants.
	template <>
	struct constants<float> : impl::floating_point_constants<float>
	{};

	/// \brief	`double` constants.
	template <>
	struct constants<double> : impl::floating_point_constants<double>
	{};

	/// \brief	`long double` constants.
	template <>
	struct constants<long double> : impl::floating_point_constants<long double>
	{};

#if MUU_HAS_FLOAT16
	/// \brief	`_Float16` constants.
	template <>
	struct constants<_Float16> : impl::floating_point_constants<_Float16>
	{};
#endif

#if MUU_HAS_FLOAT128
	/// \brief	`float128_t` constants.
	template <>
	struct constants<float128_t> : impl::floating_point_constants<float128_t>
	{};
#endif

	/// \brief	`char` constants.
	template <>
	struct constants<char> : impl::ascii_character_constants<char>
	{};

	/// \brief	`wchar_t` constants.
	template <>
	struct constants<wchar_t> : impl::ascii_character_constants<wchar_t>
	{};

#if MUU_HAS_CHAR8
	/// \brief	`char8_t` constants.
	template <>
	struct constants<char8_t> : impl::ascii_character_constants<char8_t>
	{};
#endif

	/// \brief	`char16_t` constants.
	template <>
	struct constants<char16_t> : impl::ascii_character_constants<char16_t>
	{};

	/// \brief	`char32_t` constants.
	template <>
	struct constants<char32_t> : impl::ascii_character_constants<char32_t>
	{};

	/// \brief	`signed char` constants.
	template <>
	struct constants<signed char> : impl::signed_integral_constants<signed char>
	{};

	/// \brief	`unsigned char` constants.
	template <>
	struct constants<unsigned char> : impl::unsigned_integral_constants<unsigned char>
	{};

	/// \brief	`signed short` constants.
	template <>
	struct constants<signed short> : impl::signed_integral_constants<signed short>
	{};

	/// \brief	`unsigned short` constants.
	template <>
	struct constants<unsigned short> : impl::unsigned_integral_constants<unsigned short>
	{};

	/// \brief	`signed int` constants.
	template <>
	struct constants<signed int> : impl::signed_integral_constants<signed int>
	{};

	/// \brief	`unsigned int` constants.
	template <>
	struct constants<unsigned int> : impl::unsigned_integral_constants<unsigned int>
	{};

	/// \brief	`signed long` constants.
	template <>
	struct constants<signed long> : impl::signed_integral_constants<signed long>
	{};

	/// \brief	`unsigned long` constants.
	template <>
	struct constants<unsigned long> : impl::unsigned_integral_constants<unsigned long>
	{};

	/// \brief	`signed long long` constants.
	template <>
	struct constants<signed long long> : impl::signed_integral_constants<signed long long>
	{};

	/// \brief	`unsigned long long` constants.
	template <>
	struct constants<unsigned long long> : impl::unsigned_integral_constants<unsigned long long>
	{};

#if MUU_HAS_INT128
	/// \brief	`int128_t` constants.
	template <>
	struct constants<int128_t> : impl::signed_integral_constants<int128_t>
	{};

	/// \brief	`uint128_t` constants.
	template <>
	struct constants<uint128_t> : impl::unsigned_integral_constants<uint128_t>
	{};
#endif

	/** @} */ // constants
}

MUU_POP_PRECISE_MATH;
#include "header_end.h"

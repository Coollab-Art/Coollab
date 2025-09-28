from dataclasses import dataclass, field
from typing import Optional, List, Any

# HACK: Python doesn't allow us to import from a parent folder (e.g. tooling.generate_files)
# So we need to add the path manually to sys.path
import os
import sys
from pathlib import Path

sys.path.append(
    os.path.join(
        Path(os.path.abspath(__file__)).parent.parent.parent,
        "Cool/src/Cool/ColorSpaces",
    )
)
# End of HACK

import generator_colors


@dataclass
class Conversion:
    from_: str
    to: str
    implementation: str
    intermediate_conversions: List[str] = field(default_factory=list)


@dataclass
class PrimitiveType:
    cpp: str
    user_facing_name: str
    corresponding_input_types: List[str]
    glsl: str
    parsed_from: Optional[str]


def all_primitive_types():
    ###
    # NB: The order of definition is important; it corresponds to the order the types will appear in in the dropdown of template node
    ###
    res = [
        PrimitiveType(
            cpp="Bool",
            user_facing_name="Bool",
            corresponding_input_types=["bool"],
            glsl="bool",
            parsed_from="bool",
        ),
        PrimitiveType(
            cpp="Int",
            user_facing_name="Int",
            corresponding_input_types=["int"],
            glsl="int",
            parsed_from="int",
        ),
        PrimitiveType(
            cpp="Float",
            user_facing_name="Float",
            corresponding_input_types=["float", "Cool::TimeSpeed", "Cool::Time"],
            glsl="float",
            parsed_from="float",
        ),
        PrimitiveType(
            cpp="Float_PremultipliedA",
            user_facing_name="Float, Premultiplied Alpha",
            corresponding_input_types=[],
            glsl="vec2",
            parsed_from="Float_PremultipliedA",
        ),
        PrimitiveType(
            cpp="Float_StraightA",
            user_facing_name="Float, Straight Alpha",
            corresponding_input_types=[],
            glsl="vec2",
            parsed_from="Float_StraightA",
        ),
        PrimitiveType(
            cpp="Vec2",
            user_facing_name="2D Vector",
            corresponding_input_types=["glm::vec2"],
            glsl="vec2",
            parsed_from="vec2",
        ),
        PrimitiveType(
            cpp="Vec3",
            user_facing_name="3D Vector",
            corresponding_input_types=["glm::vec3"],
            glsl="vec3",
            parsed_from="vec3",
        ),
        PrimitiveType(
            cpp="Vec4",
            user_facing_name="4D Vector",
            corresponding_input_types=["glm::vec4"],
            glsl="vec4",
            parsed_from="vec4",
        ),
        PrimitiveType(
            cpp="Mat2",
            user_facing_name="2D Matrix",
            corresponding_input_types=["glm::mat2"],
            glsl="mat2",
            parsed_from="mat2",
        ),
        PrimitiveType(
            cpp="Mat3",
            user_facing_name="3D Matrix",
            corresponding_input_types=["glm::mat3"],
            glsl="mat3",
            parsed_from="mat3",
        ),
        PrimitiveType(
            cpp="Mat4",
            user_facing_name="4D Matrix",
            corresponding_input_types=["glm::mat4"],
            glsl="mat4",
            parsed_from="mat4",
        ),
        PrimitiveType(
            cpp="UV",
            user_facing_name="UV",
            corresponding_input_types=["Cool::Point2D"],
            glsl="vec2",
            parsed_from="UV",
        ),
        PrimitiveType(
            cpp="SignedDistance",
            user_facing_name="Signed Distance",
            corresponding_input_types=[],
            glsl="float",
            parsed_from="SignedDistance",
        ),
        PrimitiveType(
            cpp="Angle",
            user_facing_name="Angle",
            corresponding_input_types=["Cool::Angle"],
            glsl="float",
            parsed_from="Angle",
        ),
        PrimitiveType(
            cpp="Hue",
            user_facing_name="Hue",
            corresponding_input_types=["Cool::Hue"],
            glsl="float",
            parsed_from="Hue",
        ),
        PrimitiveType(
            cpp="Direction2D",
            user_facing_name="Direction 2D",
            corresponding_input_types=["Cool::Direction2D"],
            glsl="vec2",
            parsed_from="Direction2D",
        ),
        PrimitiveType(
            cpp="Particle2D",
            user_facing_name="Particle 2D",
            corresponding_input_types=[],
            glsl="Particle2D",
            parsed_from="Particle2D",
        ),
        PrimitiveType(
            cpp="Particle3D",
            user_facing_name="Particle 3D",
            corresponding_input_types=[],
            glsl="Particle3D",
            parsed_from="Particle3D",
        ),
        PrimitiveType(
            cpp="Void",
            user_facing_name="Void",
            corresponding_input_types=[],
            glsl="void",
            parsed_from=None,
        ),
    ]

    res.extend(primitive_types_for_color_spaces())

    return res


def all_conversions():
    return add_float_with_alpha_conversions(
        [
            Conversion(
                from_="Float",
                to="Angle",
                implementation="""
                float FUNCTION_NAME(float x)
                {
                    return x * TAU;
                }
                """,
            ),
            Conversion(
                from_="Angle",
                to="Float",
                implementation="""
                float FUNCTION_NAME(float angle)
                {
                    return angle / TAU;
                }
            """,
            ),
            Conversion(
                from_="Float",
                to="Hue",
                implementation="""
                float FUNCTION_NAME(float x)
                {
                    return x; // No need to do anything for this conversion, the difference is purely semantic.
                }
                """,
            ),
            Conversion(
                from_="Hue",
                to="Float",
                implementation="""
                float FUNCTION_NAME(float x)
                {
                    return x; // No need to do anything for this conversion, the difference is purely semantic.
                }
                """,
            ),
            Conversion(
                from_="Float",
                to="Int",
                implementation="""
                int FUNCTION_NAME(float x)
                {
                    return int(floor(x));
                }
            """,
            ),
            Conversion(
                from_="Int",
                to="Float",
                implementation="""
                float FUNCTION_NAME(int x)
                {
                    return float(x);
                }
            """,
            ),
            Conversion(
                from_="Float",
                to="Bool",
                implementation="""
                bool FUNCTION_NAME(float x)
                {
                    return x > 0.5;
                }
            """,
            ),
            Conversion(
                from_="Bool",
                to="Float",
                implementation="""
                float FUNCTION_NAME(bool b)
                {
                    return b ? 1. : 0.;
                }
            """,
            ),
            Conversion(
                from_="Angle",
                to="Direction2D",
                implementation="""
                vec2 FUNCTION_NAME(float angle)
                {
                    return vec2(cos(angle), sin(angle));
                }
            """,
            ),
            Conversion(
                from_="Float",
                to="Direction2D",
                implementation="""
                vec2 FUNCTION_NAME(float x)
                {
                    float angle = x * TAU;
                    return vec2(cos(angle), sin(angle));
                }
            """,
            ),
            Conversion(
                from_="Direction2D",
                to="Angle",
                implementation="""
                float FUNCTION_NAME(vec2 dir)
                {
                    return dir.x != 0.f
                                ? atan(dir.y, dir.x)
                                : dir.y > 0.
                                    ? PI / 2.
                                    : -PI / 2.;
                }
            """,
            ),
            Conversion(
                from_="UV",
                to="Vec2",
                implementation="""
                vec2 FUNCTION_NAME(vec2 x)
                {
                    return x; // No need to do anything for this conversion, the difference is purely semantic.
                }
                """,
            ),
            Conversion(
                from_="Vec2",
                to="UV",
                implementation="""
                vec2 FUNCTION_NAME(vec2 x)
                {
                    return x; // No need to do anything for this conversion, the difference is purely semantic.
                }
                """,
            ),
            Conversion(
                from_="Void",
                to="UV",
                implementation="""
                vec2 FUNCTION_NAME/*needs_coollab_context*/()
                {
                    return coollab_context.uv;
                }
            """,
            ),
            Conversion(
                from_="Void",
                to="Particle2D",
                implementation="""
                Particle2D FUNCTION_NAME/*needs_coollab_context*/()
                {
                    return coollab_context.particle;
                }
            """,
            ),
            Conversion(
                from_="Void",
                to="Particle3D",
                implementation="""
                Particle3D FUNCTION_NAME/*needs_coollab_context*/()
                {
                    return coollab_context.particle;
                }
            """,
            ),
        ]
    )


def add_float_with_alpha_conversions(conversions: List[Conversion]) -> List[Conversion]:
    for conversion in conversions:
        if conversion.from_ == "Float":
            for alpha_space in non_null_alpha_spaces():
                conversions.append(
                    Conversion(
                        from_=f"Float{alpha_space}",
                        to=conversion.to,
                        implementation=f"""
                        {glsl_type_from_cpp_type(conversion.to)} FUNCTION_NAME(vec2 x)
                        {{
                            return Coollab_{conversion.to}_from_Float(Coollab_Float_from_Float{alpha_space}(x));
                        }}
                    """,
                        intermediate_conversions=[
                            f"Coollab_{conversion.to}_from_Float",
                            f"Coollab_Float_from_Float{alpha_space}",
                        ],
                    )
                )
        elif conversion.to == "Float":
            for alpha_space in non_null_alpha_spaces():
                conversions.append(
                    Conversion(
                        from_=conversion.from_,
                        to=f"Float{alpha_space}",
                        implementation=f"""
                        vec2 FUNCTION_NAME({glsl_type_from_cpp_type(conversion.from_)} x)
                        {{
                            return Coollab_Float{alpha_space}_from_Float(Coollab_Float_from_{conversion.from_}(x));
                        }}
                    """,
                        intermediate_conversions=[
                            f"Coollab_Float{alpha_space}_from_Float",
                            f"Coollab_Float_from_{conversion.from_}",
                        ],
                    )
                )
    return conversions


def glsl_type_from_cpp_type(cpp_type: str):
    for type in all_primitive_types():
        if type.cpp == cpp_type:
            return type.glsl


def color_spaces():
    import generator_colors

    return generator_colors.color_spaces()


def color_and_greyscale_spaces():
    spaces = color_spaces()
    spaces.append(
        generator_colors.ColorSpace(
            name_in_code="Float",
            user_facing_name="Float",
        ),
    )
    return spaces


def primitive_types_for_color_spaces() -> List[PrimitiveType]:
    res = []

    for color_space in color_spaces():
        # No alpha
        res.append(
            PrimitiveType(
                cpp=color_space.name_in_code,
                user_facing_name=color_space.user_facing_name,
                corresponding_input_types=["Cool::Color"],
                glsl="vec3",
                parsed_from=color_space.name_in_code,
            )
        )
        # Premultiplied alpha
        res.append(
            PrimitiveType(
                cpp=color_space.name_in_code + "_PremultipliedA",
                user_facing_name=color_space.user_facing_name + ", Premultiplied Alpha",
                corresponding_input_types=["Cool::ColorAndAlpha"],
                glsl="vec4",
                parsed_from=color_space.name_in_code + "_PremultipliedA",
            )
        )
        # Straight alpha
        res.append(
            PrimitiveType(
                cpp=color_space.name_in_code + "_StraightA",
                user_facing_name=color_space.user_facing_name + ", Straight Alpha",
                corresponding_input_types=["Cool::ColorAndAlpha"],
                glsl="vec4",
                parsed_from=color_space.name_in_code + "_StraightA",
            )
        )

    return res


def alpha_spaces():
    return ["", "_StraightA", "_PremultipliedA"]


def non_null_alpha_spaces():
    l = alpha_spaces()
    l.remove("")
    return l


def vec_type(dimension: int):
    if dimension == 1:
        return "float"
    return f"vec{dimension}"


def dimension(color: generator_colors.ColorSpace):
    if color.name_in_code == "Float":
        return 1
    return 3


def implicit_color_conversions_impl():
    from itertools import product

    res = ["", ""]
    for color1, color2 in product(
        color_and_greyscale_spaces(), color_and_greyscale_spaces()
    ):
        for alpha1, alpha2 in product(alpha_spaces(), alpha_spaces()):
            type1 = color1.name_in_code + alpha1
            type2 = color2.name_in_code + alpha2
            if type1 == type2:
                continue

            def gen_code(in_vec: str, out_vec: str, implementation: str):
                nonlocal res
                function_name = f"Coollab_{type2}_from_{type1}"
                res[
                    0
                ] += f"""
                    static auto gen_{function_name}() -> Function
                    {{
                        return {{
                            .name       = "{function_name}",
                            .definition = R"STR(
                                {out_vec} {function_name}({in_vec} from)
                                {{
                                    {implementation}
                                }}
                                )STR",
                        }};
                    }}
                """
                res[
                    1
                ] += f"""
                if (from == PrimitiveType::{type1} && to == PrimitiveType::{type2})
                {{
                    return context.push_function(gen_{function_name}());
                }}
                """

            color_conversion = (
                f"Cool_{color2.name_in_code}_from_{color1.name_in_code}"
                if color1 != color2
                else ""
            )
            color_components = "xyz" if color1.name_in_code != "Float" else "x"
            alpha_component = "a" if color1.name_in_code != "Float" else "y"
            color_or_greyscale_from = (
                "color" if color1.name_in_code != "Float" else "greyscale"
            )
            color_or_greyscale_to = (
                "color" if color2.name_in_code != "Float" else "greyscale"
            )
            match alpha1, alpha2:
                case "", "":
                    gen_code(
                        in_vec=vec_type(dimension(color1)),
                        out_vec=vec_type(dimension(color2)),
                        implementation=f"""
                        {vec_type(dimension(color2))} to = {color_conversion}(from);
                        return to;
                    """,
                    )
                case "_StraightA", "":
                    gen_code(
                        in_vec=vec_type(dimension(color1) + 1),
                        out_vec=vec_type(dimension(color2)),
                        implementation=f"""
                        {vec_type(dimension(color2))} to = {color_conversion}(Cool_premultiply_{color_or_greyscale_from}(from.{color_components}, from.{alpha_component}));
                        return to;
                    """,
                    )
                case "_PremultipliedA", "":
                    gen_code(
                        in_vec=vec_type(dimension(color1) + 1),
                        out_vec=vec_type(dimension(color2)),
                        implementation=f"""
                        {vec_type(dimension(color2))} to = {color_conversion}(from.{color_components});
                        return to;
                    """,
                    )
                case "", "_StraightA":
                    gen_code(
                        in_vec=vec_type(dimension(color1)),
                        out_vec=vec_type(dimension(color2) + 1),
                        implementation=f"""
                        {vec_type(dimension(color2))} to = {color_conversion}(from);
                        return {vec_type(dimension(color2)+1)}(to, 1.);
                    """,
                    )
                case "", "_PremultipliedA":
                    gen_code(
                        in_vec=vec_type(dimension(color1)),
                        out_vec=vec_type(dimension(color2) + 1),
                        implementation=f"""
                        {vec_type(dimension(color2))} to = {color_conversion}(from);
                        return {vec_type(dimension(color2)+1)}(to, 1.);
                    """,
                    )
                case "_StraightA", "_StraightA":
                    gen_code(
                        in_vec=vec_type(dimension(color1) + 1),
                        out_vec=vec_type(dimension(color2) + 1),
                        implementation=f"""
                        {vec_type(dimension(color2))} to = {color_conversion}(from.{color_components});
                        return Cool_apply_straight_alpha_to_{color_or_greyscale_to}(to, from.{alpha_component});
                    """,
                    )
                case "_PremultipliedA", "_PremultipliedA":
                    gen_code(
                        in_vec=vec_type(dimension(color1) + 1),
                        out_vec=vec_type(dimension(color2) + 1),
                        implementation=f"""
                        // We need to unpremultiply for the color conversion, and re-premultiply afterwards
                        {vec_type(dimension(color2))} to = {color_conversion}(Cool_unpremultiply_{color_or_greyscale_from}(from.{color_components}, from.{alpha_component}));
                        return Cool_apply_premultiplied_alpha_to_{color_or_greyscale_to}(to, from.{alpha_component});
                    """,
                    )
                case "_PremultipliedA", "_StraightA":
                    gen_code(
                        in_vec=vec_type(dimension(color1) + 1),
                        out_vec=vec_type(dimension(color2) + 1),
                        implementation=f"""
                        {vec_type(dimension(color2))} to = {color_conversion}(Cool_unpremultiply_{color_or_greyscale_from}(from.{color_components}, from.{alpha_component}));
                        return Cool_apply_straight_alpha_to_{color_or_greyscale_to}(to, from.{alpha_component});
                    """,
                    )
                case "_StraightA", "_PremultipliedA":
                    gen_code(
                        in_vec=vec_type(dimension(color1) + 1),
                        out_vec=vec_type(dimension(color2) + 1),
                        implementation=f"""
                        {vec_type(dimension(color2))} to = {color_conversion}(from.{color_components});
                        return Cool_apply_premultiplied_alpha_to_{color_or_greyscale_to}(to, from.{alpha_component});
                    """,
                    )
    return res


def implicit_color_conversions():
    return implicit_color_conversions_impl()[1]


def def_implicit_color_conversions():
    return implicit_color_conversions_impl()[0]


def has_an_alpha_channel():
    res = ""
    for color_space in color_and_greyscale_spaces():
        res += f"case PrimitiveType::{color_space.name_in_code}_StraightA:\n"
        res += f"case PrimitiveType::{color_space.name_in_code}_PremultipliedA:\n"

    res += "return true;"
    return res


def has_straight_alpha_channel():
    res = ""
    for color_space in color_and_greyscale_spaces():
        res += f"case PrimitiveType::{color_space.name_in_code}_StraightA:\n"

    res += "return true;"
    return res


def with_straight_alpha():
    res = ""
    for color_space in color_and_greyscale_spaces():
        res += f"""
case PrimitiveType::{color_space.name_in_code}:
    return PrimitiveType::{color_space.name_in_code}_StraightA;"""

    return res


def with_straight_alpha_if_has_no_alpha():
    res = ""
    for color_space in color_and_greyscale_spaces():
        res += f"""
case PrimitiveType::{color_space.name_in_code}:
    return PrimitiveType::{color_space.name_in_code}_StraightA;"""

    return res


def is_color_type():
    from itertools import product

    def code(color_space, alpha_space):
        return f"type == PrimitiveType::{color_space.name_in_code}{alpha_space}"

    return "\n ||".join(
        map(lambda x: code(x[0], x[1]), product(color_spaces(), alpha_spaces()))
    )


def is_greyscale_type():
    from itertools import product

    def code(alpha_space):
        return f"type == PrimitiveType::Float{alpha_space}"

    return "\n ||".join(map(code, alpha_spaces()))


def primitive_types_enum_members():
    from pipe import map

    return ",\n".join(all_primitive_types() | map(lambda type: f"{type.cpp}"))


def glsl_type_as_string_cases():
    from pipe import map

    return "\n".join(
        all_primitive_types()
        | map(
            lambda type: f'case PrimitiveType::{type.cpp}: return "/*{type.cpp}*/ {type.glsl}";'
        )
    )


def raw_glsl_type_as_string_cases():
    from pipe import map

    return "\n".join(
        all_primitive_types()
        | map(lambda type: f'case PrimitiveType::{type.cpp}: return "{type.glsl}";')
    )


def cpp_type_as_string_cases():
    from pipe import map

    return "\n".join(
        all_primitive_types()
        | map(lambda type: f'case PrimitiveType::{type.cpp}: return "{type.cpp}";')
    )


def check_that_each_primitive_type_corresponds_to_a_different_input_type():
    import collections
    from pipe import chain, map

    input_types = list(
        all_primitive_types() | map(lambda type: type.corresponding_input_types) | chain
    )

    duplicates = [
        item
        for item, count in collections.Counter(input_types).items()
        if count > 1 and item != "Cool::Color" and item != "Cool::ColorAndAlpha"
    ]

    if len(duplicates) > 0:
        raise Exception(
            f"You can't have two primitive types corresponding to the same input type ({', '.join(duplicates)})"
        )


def check_color_space(type):
    if type[0] == "Cool::Color":
        return f"&& std::get<Cool::SharedVariable<{type[0]}>>(var).get_ref().desired_color_space == static_cast<int>(Cool::ColorSpace::{type[1]})"
    elif type[0] == "Cool::ColorAndAlpha":
        return f"&& std::get<Cool::SharedVariable<{type[0]}>>(var).get_ref().desired_color_space == static_cast<int>(Cool::ColorAndAlphaSpace::{type[1]})"
    else:
        return ""


def variable_to_primitive_type():
    from pipe import map, chain

    check_that_each_primitive_type_corresponds_to_a_different_input_type()
    return "\n".join(
        all_primitive_types()
        | map(
            lambda type: type.corresponding_input_types | map(lambda t: [t, type.cpp])
        )
        | chain
        | map(
            lambda type: f"""
if (std::holds_alternative<Cool::SharedVariable<{type[0]}>>(var) {check_color_space(type)})
    return PrimitiveType::{type[1]};"""
        )
    )


def parse_primitive_type():
    from pipe import map, where

    return "\n".join(
        all_primitive_types()
        | where(lambda x: x.parsed_from is not None)
        | map(
            lambda type: f"""if (str == "{type.parsed_from}") {{ return PrimitiveType::{type.cpp}; }}"""
        )
    )


def string_listing_the_parsed_types():
    from pipe import map, where

    return (
        'R"STR('
        + "\n".join(
            all_primitive_types()
            | where(lambda x: x.parsed_from is not None)
            | map(lambda type: f"  - {type.parsed_from}")
        )
        + ')STR"'
    )


def def_implicit_conversions():
    def replace_function_name(code: str, name: str):
        return code.replace("FUNCTION_NAME", name)

    def gen_conversion(conversion: Conversion):
        function_name = f"Coollab_{conversion.to}_from_{conversion.from_}"
        return f"""
            static auto gen_{function_name}() -> Function
            {{
                return {{
                    .name       = "{function_name}",
                    .definition = R"STR({replace_function_name(conversion.implementation, function_name)})STR",
                }};
            }}
        """

    from pipe import map

    return "\n".join(all_conversions() | map(gen_conversion))


def implicit_conversions():
    def gen_conversion(conversion: Conversion):
        function_name = f"Coollab_{conversion.to}_from_{conversion.from_}"
        return f"""
            if (from == PrimitiveType::{conversion.from_} && to == PrimitiveType::{conversion.to})
            {{
                {";\n".join([f"context.push_function(gen_{intermediate}())" for intermediate in conversion.intermediate_conversions])};
                return context.push_function(gen_{function_name}());
            }}
        """

    from pipe import map

    return "\n".join(all_conversions() | map(gen_conversion))


def can_convert():
    def code(conversion: Conversion):
        return f"""
            if (from == PrimitiveType::{conversion.from_} && to == PrimitiveType::{conversion.to})
                return true;
        """

    from pipe import map

    return "\n".join(all_conversions() | map(code))


if __name__ == "__main__":
    # HACK: Python doesn't allow us to import from a parent folder (e.g. tooling.generate_files)
    # So we need to add the path manually to sys.path
    import os
    import sys
    from pathlib import Path

    sys.path.append(
        os.path.join(Path(os.path.abspath(__file__)).parent.parent.parent, "tooling")
    )
    # End of HACK
    import generate_files

    generate_files.generate(
        folder="generated",
        files=[
            primitive_types_enum_members,
            glsl_type_as_string_cases,
            raw_glsl_type_as_string_cases,
            cpp_type_as_string_cases,
            variable_to_primitive_type,
            parse_primitive_type,
            string_listing_the_parsed_types,
            implicit_color_conversions,
            def_implicit_color_conversions,
            has_an_alpha_channel,
            has_straight_alpha_channel,
            with_straight_alpha,
            with_straight_alpha_if_has_no_alpha,
            is_color_type,
            is_greyscale_type,
            implicit_conversions,
            def_implicit_conversions,
            can_convert,
        ],
    )

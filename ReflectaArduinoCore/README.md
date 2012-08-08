# ReflectaArduinoCore #

Reflecta Arduino Core binds the functions pinMode, digitalRead, digitalWrite, analogRead, and analogWrite to the ARDU1 interface.  To use, QueryInterface 'ARDU1' and this will give you the function id of pinMode.  Increase the function id for each following function in the interface.  For example, if QueryInterface ARDU1 returns 9, then function id 9 = pinMode and function id 12 = analogRead.

pinMode, digitalWrite, and analogWrite take two byte parameters, one byte for each function parameter.  digitalRead and analogRead take one byte parameters.  For example, when QueryInterface ARDU1 = 9, sending

> 0x09 02 11 1

> Function Id = 0x09, ParameterLength = 2, Parameter[0] = 11, Parameter[1] = 1

would call

> pinMode(11, 1);
